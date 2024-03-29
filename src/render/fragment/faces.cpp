#include "render/fragment.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/matrix2d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "engine.h"
#include "lines.h"
#include "easy_image.h"
#include "render/aabb.h"
#include "render/geometry.h"
#include "render/rect.h"

/** If something looks off (vs examples), try changing these values **/

// Cursus says 1.0001, but a little lower gives better shadow quality & seems
// to be consistent with the example images
#define Z_BIAS (1.00001)
// Ditto
#define Z_SHADOW_BIAS (1.5e-6)

using namespace std;

namespace engine {
namespace render {

/**
 * \brief Apply specular light.
 */
static ALWAYS_INLINE optional<Color> specular(const TriangleFigure &f, Color c, double dot, Vector3D n, Vector3D cam_dir, Vector3D direction) {
	auto r = 2 * dot * n + direction;
	auto rdot = r.dot(-cam_dir);
	if (rdot > 0) {
		double v = f.reflection_int != numeric_limits<unsigned int>::max()
			? pow_uint(rdot, f.reflection_int)
			: pow(rdot, f.reflection);
		return f.specular * c * v;
	}
	return optional<Color>();
}

/**
 * \brief Apply directional light.
 */
static ALWAYS_INLINE optional<Color> directional_light(const TriangleFigure &f, const DirectionalLight &light, Vector3D n, Vector3D cam_dir) {
	auto dot = n.dot(-light.direction);
	if (dot > 0) {
		// Diffuse
		auto color = f.diffuse * light.diffuse * dot;
		// Specular
		auto s = specular(f, light.specular, dot, n, cam_dir, light.direction);
		if (s.has_value()) {
			color += *s;
		}
		return optional(color);
	}
	return optional<Color>();
}

/**
 * \brief Determine if a shadow is cast at the given point.
 */
static ALWAYS_INLINE bool shadowed(const PointLight &p, Point3D point) {
	auto l = point * p.cached.eye;
	auto lx = l.x / -l.z * p.cached.d + p.cached.offset.x;
	auto ly = l.y / -l.z * p.cached.d + p.cached.offset.y;
	assert(!isinf(lx) && !isnan(lx));
	assert(!isinf(ly) && !isnan(ly));
	auto fx = floor(lx);
	auto fy = floor(ly);
	auto cx = fx + 1;
	auto cy = fy + 1;
	auto get_z = [&p](unsigned int x, unsigned int y) {
		return x < p.cached.zbuf.get_width() && y < p.cached.zbuf.get_height()
			? ((const ZBuffer &)p.cached.zbuf)(x, y)
			: numeric_limits<double>::infinity();
	};
	auto cxa = lx - fx;
	auto cya = ly - fy;
	auto fxa = 1 - cxa;
	auto fya = 1 - cya;
	assert(1 >= fxa && fxa >= 0);
	assert(1 >= fya && fya >= 0);
	assert(1 >= cxa && cxa >= 0);
	assert(1 >= cya && cya >= 0);
	auto inv_z = (
		(
			+ get_z(fx, fy) * fxa
			+ get_z(cx, fy) * cxa
		) * fya + (
			+ get_z(fx, cy) * fxa
			+ get_z(cx, cy) * cxa
		) * cya
	);
	assert(!isnan(inv_z) && "shadow 1/z is NaN");

	return inv_z + Z_SHADOW_BIAS < 1 / l.z;
}

/**
 * \brief Apply point light.
 */
static ALWAYS_INLINE optional<Color> point_light(const TriangleFigure &f, const PointLight &light, Point3D point, bool shadows, Vector3D n, Vector3D cam_dir) {
	auto direction = (point - light.point).normalize();
	auto dot = n.dot(-direction);
	if (dot > 0) {
		// Check if shadowed
		if (shadows && shadowed(light, point)) {
			return optional<Color>();
		}
		// Diffuse
		auto color = f.diffuse * light.diffuse * max(1 - (1 - dot) / (1 - light.spot_angle_cos), 0.0);
		// Specular
		auto s = specular(f, light.specular, dot, n, cam_dir, direction);
		if (s.has_value()) {
			color += *s;
		}
		return optional(color);
	}
	return optional<Color>();
}

/**
 * \brief Determine P and Q interpolation factors for BA and CA respectively for
 * a triangle ABC.
 */
static ALWAYS_INLINE Vector2D calc_pq(const TriangleFigure &f, Face t, Point3D point) {
	return calc_pq(f.points[t.a], f.points[t.b], f.points[t.c], point);
}

/**
 * \brief Get the color at a point from an associated texture.
 */
static ALWAYS_INLINE Color texture_color(const TriangleFigure &f, Face t, Vector2D pq) {
	auto uv = interpolate(f.uv[t.a].to_vector(), f.uv[t.b].to_vector(), f.uv[t.c].to_vector(), pq);
	return Color(f.texture.value().get_clamped(uv));
}

/**
 * \brief Get the color of a cubemap texture at a pixel
 */
static ALWAYS_INLINE Color cubemap_color(
	const Lights &lights,
	Point3D point,
	Vector3D normal
) {
	// Current layout of cubemap:
	//
	//   T
	//   F R B L
	//   B
	//
	// Note that -Z is forward, Y is top and X is right
	Point2D uv;

	Aabb aabb {
		Vector3D(-1,-1,-1) * lights.cubemap_size,
		Vector3D(1,1,1) * lights.cubemap_size,
	};
	normal *= lights.inv_eye;
	point *= lights.inv_eye;
	auto f3 = (
		normal.sign().max(Vector3D()) * aabb.max.to_vector()
		- normal.sign().min(Vector3D()) * aabb.min.to_vector()
		- point.to_vector()
	) / normal;

	double f = f3.abs().min();
	point += normal * f;

	auto p3 = (point - aabb.min) / aabb.size();

	Vector2D p;
	if (f == f3.abs().x) {
		// Back / front
		uv = { normal.x < 0 ? 0.75 : 0.25, 1.0 / 3 };
		p = { normal.x < 0 ? 1.0 - p3.y : p3.y, p3.z };
	} else if (f == f3.abs().y) {
		// Right / left
		uv = { normal.y > 0 ? 0.5 : 0, 1.0 / 3 };
		p = { normal.y > 0 ? 1.0 - p3.x : p3.x, p3.z };
	} else {
		// Top / bottom
		uv = { 0.25, normal.z > 0 ? 2.0 / 3 : 0 };
		p = { p3.y, normal.z > 0 ? 1.0 - p3.x : p3.x };
	}

	p.x /= 4;
	p.y /= 3;

	return Color(lights.cubemap->get_clamped(uv + p));
}

void draw(const std::vector<TriangleFigure> &figures, const Lights &lights, double d, Vector2D offset, img::EasyImage &img, TaggedZBuffer &zbuf) {
	struct Tri {
		Point3D a, b, c;
	};
	auto f2p = [](auto &f, auto &t) {
		return Tri {
			f.points[t.a],
			f.points[t.b],
			f.points[t.c],
		};
	};

	// Process point light shadows first
	if (lights.shadows) {
		auto &inv_project = lights.inv_eye;

		for (size_t pi = 0; pi < lights.point.size(); pi++) {
			auto &p = lights.point[pi];

			// Project from camera perspective & determine bounds
			auto pt = p.point * inv_project;
			p.cached.eye = inv_project * look_direction(pt, -(pt - Point3D()));

			vector<ZBufferTriangleFigure> zfigs = lights.zfigures;

			Rect rect;
			rect.min.x = rect.min.y = +numeric_limits<double>::infinity();
			rect.max.x = rect.max.y = -numeric_limits<double>::infinity();
			for (auto &f : zfigs) {
				for (auto &a : f.points) {
					a *= p.cached.eye;
					rect |= project(a);
				}
			}

			// Create ZBuffer
			{
				Vector2D dim;
				calc_image_parameters(rect, lights.shadow_mask, p.cached.d, p.cached.offset, dim);
				p.cached.zbuf = ZBuffer(round_up(dim.x), round_up(dim.y));
			}

			// Fill in ZBuffer with figure & triangle IDs
			assert(figures.size() < UINT16_MAX);
			for (u_int16_t i = 0; i < zfigs.size(); i++) {
				auto &f = zfigs[i];
				assert(f.faces.size() < UINT32_MAX);
				for (u_int32_t k = 0; k < f.faces.size(); k++) {
					auto &t = f.faces[k];
					auto a = f.points[t.a], b = f.points[t.b], c = f.points[t.c];
					auto norm = (b - a).cross(c - a);
					if (!f.can_cull || norm.dot(a - Point3D()) <= 0) {
						p.cached.zbuf.triangle(a, b, c, p.cached.d, p.cached.offset, 1);
					}
				}
			}
		}
	}

	// Fill in ZBuffer with figure & triangle IDs
	assert(figures.size() < UINT16_MAX);
	for (u_int16_t i = 0; i < figures.size(); i++) {
		auto &f = figures[i];
		assert(f.faces.size() < UINT32_MAX);
		for (u_int32_t k = 0; k < f.faces.size(); k++) {
			auto &t = f.faces[k];
			auto abc = f2p(f, t);
			auto a = abc.a, b = abc.b, c = abc.c;
#if GRAPHICS_DEBUG_Z == 2 || GRAPHICS_DEBUG_FACES == 2
			{
#else
			if (!f.flags.can_cull() || (b -a).cross(c - a).dot(a - Point3D()) <= 0) {
#endif
				zbuf.triangle(a, b, c, d, offset, {i, k, NAN}, Z_BIAS);
			}
		}
	}

#if GRAPHICS_DEBUG > 0 || GRAPHICS_DEBUG_NORMALS > 0 || GRAPHICS_DEBUG_FACES > 0
	// "Randomize" face colors to help debug clipping & other issues
	const img::Color colors_pool[12] = {
		img::Color(255, 0, 0),
		img::Color(0, 255, 0),
		img::Color(0, 0, 255),

		img::Color(0, 255, 255),
		img::Color(255, 0, 255),
		img::Color(255, 255, 0),

		img::Color(50, 200, 200),
		img::Color(200, 50, 200),
		img::Color(200, 200, 50),

		img::Color(200, 50, 50),
		img::Color(50, 200, 50),
		img::Color(50, 50, 200),
	};
	const size_t color_pool_size = sizeof(colors_pool) / sizeof(*colors_pool);
#endif

#if GRAPHICS_DEBUG_Z > 0
	auto &_dbg_zb = zbuf;
#if GRAPHICS_DEBUG_Z == 2
	for (auto &p : lights.point) {
		_dbg_zb = p.cached.zbuf;
		break;
	}
#endif
	auto max_inv_z = -numeric_limits<double>::infinity();
	auto min_inv_z = numeric_limits<double>::infinity();
	for (unsigned int y = 0; y < _dbg_zb.get_height(); y++) {
		for (unsigned int x = 0; x < _dbg_zb.get_width(); x++) {
			auto inv_z = _dbg_zb.get(x, y).inv_z;
			if (isinf(inv_z))
				continue;
			max_inv_z = max(max_inv_z, inv_z);
			min_inv_z = min(min_inv_z, inv_z);
		}
	}
#endif

	// Draw triangle colors
	// A custom iterator would be neat but C++'s iterators are cursed so nah.
	for (unsigned int y = 0; y < img.get_height(); y++) {
		for (unsigned int x = 0; x < img.get_width(); x++) {
			auto pair = zbuf.get(x, y);
			Point3D point;
			Vector3D n;
			Color color;
			if (pair.is_valid()) {
				auto &f = figures[pair.figure_id];
				auto &t = f.faces[pair.triangle_id];

				// Invert perspective projection
				// Given: x', y', 1/z, dx, dy
				// x' = x / -z * d + dx => x = (x' - dx) * -z / d, ditto for y
				point = {
					(x - offset.x) / (d * -pair.inv_z),
					(y - offset.y) / (d * -pair.inv_z),
					1 / pair.inv_z
				};

				auto cam_dir = (point - Point3D()).normalize();

				auto pq = calc_pq(f, t, point);

				if (f.flags.separate_normals()) {
					n = interpolate(f.normals[t.a], f.normals[t.b], f.normals[t.c], pq);
					n = n.normalize();
					if (f.flags.clipped()) {
						auto m = f2p(f, t);
						n = (m.b - m.a).cross(m.c - m.a).dot(cam_dir) > 0 ? -n : n;
					}
				} else if (!f.normals.empty()) {
					n = f.normals[pair.triangle_id];
					if (f.flags.clipped()) {
						n = n.dot(cam_dir) > 0 ? -n : n;
					}
				}

				color = f.ambient * lights.ambient;
#if GRAPHICS_DEBUG_Z > 0
				color = Color();
#endif

				for (auto &d : lights.directional) {
					auto c = directional_light(f, d, n, cam_dir);
					if (c.has_value()) {
						color += *c;
					}
				}
				for (auto &p : lights.point) {
					auto c = point_light(f, p, point, lights.shadows, n, cam_dir);
					if (c.has_value()) {
						color += *c;
					}
				}

				if (f.texture.has_value()) {
					color *= texture_color(f, f.faces[pair.triangle_id], pq);
				}

#if GRAPHICS_DEBUG_FACES == 2
				auto cg = (color.r + color.g + color.b) / 3;
				color = (f.points[t.b] - f.points[t.a]).cross(f.points[t.c] - f.points[t.a]).dot(cam_dir) > 0
					? Color(cg, 0, 0)
					: Color(0, cg, 0);
#elif GRAPHICS_DEBUG_FACES > 0
				color = Color(colors_pool[pair.triangle_id % color_pool_size]);
#endif
			} else if (lights.cubemap.has_value()) {
				// Draw cubemap background (skybox)
				point = Point3D() * lights.eye;
				n = { (x - offset.x) / d, (y - offset.y) / d, -1 };
				color = { 1, 1, 1 };
			}

			if (lights.cubemap.has_value()) {
				color *= cubemap_color(lights, point, n);
			}

#if GRAPHICS_DEBUG_Z != 2 && GRAPHICS_DEBUG_Z > 0
			color = Color(1, 1, 1) * (pair.inv_z - min_inv_z) / (max_inv_z - min_inv_z);
#endif

			assert(color.r >= 0 && "Colors can't be negative");
			assert(color.g >= 0 && "Colors can't be negative");
			assert(color.b >= 0 && "Colors can't be negative");
			img(x, y) = color.to_img_color();

		}
	}

#if GRAPHICS_DEBUG_NORMALS > 0
	for (auto &f : figures) {
		if (f.flags.separate_normals()) {
			for (size_t i = 0; i < f.points.size(); i++) {
				auto from = f.points[i];
				auto to = from + f.normals[i] * 0.1;
				Point3D ft(from.x / -from.z * d + offset.x, from.y / -from.z * d + offset.y, from.z);
				Point3D tt(to.x / -to.z * d + offset.x, to.y / -to.z * d + offset.y, to.z);
				img.draw_zbuf_line_clip(
					zbuf,
					round_up(ft.x), round_up(ft.y), ft.z,
					round_up(tt.x), round_up(tt.y), tt.z,
					{ 255, 255, 255 }
				);
			}
		} else {
			for (size_t i = 0; i < f.faces.size(); i++) {
				auto &t = f.faces[i];
				auto abc = f2p(f, t);
				auto a = abc.a, b = abc.b, c = abc.c;
				auto from = Point3D::center({ a, b, c });
				auto to = from + f.normals[i] * 0.1;
				Point3D ft(from.x / -from.z * d + offset.x, from.y / -from.z * d + offset.y, from.z);
				Point3D tt(to.x / -to.z * d + offset.x, to.y / -to.z * d + offset.y, to.z);
				img.draw_zbuf_line_clip(
					zbuf,
					round_up(ft.x), round_up(ft.y), ft.z,
					round_up(tt.x), round_up(tt.y), tt.z,
					{ 255, 255, 255 }
				);
			}
		}
	}
#endif

#if GRAPHICS_DEBUG_EDGES > 0
	for (auto &f : figures) {
		for (auto &t : f.faces) {
			auto abc = f2p(f, t);
			auto a = abc.a, b = abc.b, c = abc.c;
			Point3D la(a.x / -a.z * d + offset.x, a.y / -a.z * d + offset.y, a.z);
			Point3D lb(b.x / -b.z * d + offset.x, b.y / -b.z * d + offset.y, b.z);
			Point3D lc(c.x / -c.z * d + offset.x, c.y / -c.z * d + offset.y, c.z);
			auto f = [&](auto p, auto q) {
				img.draw_zbuf_line(
					zbuf,
					round_up(p.x), round_up(p.y), p.z - 0.0000001,
					round_up(q.x), round_up(q.y), p.z - 0.0000001,
					{ 255, 255, 255 }
				);
			};
			f(la, lb);
			f(lb, lc);
			f(lc, la);
		}
	}
#endif

#if GRAPHICS_DEBUG > 2
	// Axes
	{
		Point3D o;
		o *= lights.eye;
		auto ox = o + Vector3D(1000, 0, 0) * lights.eye;
		auto oy = o + Vector3D(0, 1000, 0) * lights.eye;
		auto oz = o + Vector3D(0, 0, 1000) * lights.eye;
		Point3D lo(o.x / -o.z * d + offset.x, o.y / -o.z * d + offset.y, o.z);
		Point3D lox(ox.x / -ox.z * d + offset.x, ox.y / -ox.z * d + offset.y, ox.z);
		Point3D loy(oy.x / -oy.z * d + offset.x, oy.y / -oy.z * d + offset.y, oy.z);
		Point3D loz(oz.x / -oz.z * d + offset.x, oz.y / -oz.z * d + offset.y, oz.z);
		Line3D(lo, lox, img::Color(255, 0, 0)).draw_clip(img, zbuf);
		Line3D(lo, loy, img::Color(0, 255, 0)).draw_clip(img, zbuf);
		Line3D(lo, loz, img::Color(0, 0, 255)).draw_clip(img, zbuf);
	}
#endif
}

img::EasyImage draw(const vector<TriangleFigure> &figures, const Lights &lights, unsigned int size, Color background) {
	if (figures.empty()) {
		return img::EasyImage(0, 0);
	}

	Rect dim;
	dim.min.x = dim.min.y = +numeric_limits<double>::infinity();
	dim.max.x = dim.max.y = -numeric_limits<double>::infinity();
	for (auto &f : figures) {
		dim |= f.bounds_projected();
	}

	double d;
	Vector2D offset;
	auto img = create_img(dim, size, background, d, offset);

	assert(!isnan(d));
	assert(!isnan(offset.x));
	assert(!isnan(offset.y));

	TaggedZBuffer zbuf(img.get_width(), img.get_height());

	draw(figures, lights, d, offset, img, zbuf);

	return img;
}

}
}
