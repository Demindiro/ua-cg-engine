#include "shapes.h"
#include <initializer_list>
#include <limits>
#include <vector>
#include "engine.h"
#include "ini_configuration.h"
#include "vector3d.h"
#include "shapes/buckyball.h"
#include "shapes/circle.h"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/dodecahedron.h"
#include "shapes/icosahedron.h"
#include "shapes/tetrahedron.h"
#include "shapes/octahedron.h"
#include "shapes/sphere.h"
#include "shapes/torus.h"
#include "wireframe.h"


using namespace std;

namespace shapes {
	Matrix transform_from_conf(ini::Section &conf, Matrix &projection) {
		auto rot_x = conf["rotateX"].as_double_or_die() * M_PI / 180;
		auto rot_y = conf["rotateY"].as_double_or_die() * M_PI / 180;
		auto rot_z = conf["rotateZ"].as_double_or_die() * M_PI / 180;
		auto center = tup_to_point3d(conf["center"].as_double_tuple_or_die());
		auto scale = conf["scale"].as_double_or_die();

		// Create transformation matrix
		// Order of operations: scale > rot_x > rot_y > rot_z > translate > project
		// NB the default constructor creates identity matrices (diagonal is 1)
		Matrix mat_scale, mat_rot_x, mat_rot_y, mat_rot_z, mat_translate;

		mat_scale(1, 1) = mat_scale(2, 2) = mat_scale(3, 3) = scale;

		mat_rot_x = Rotation(rot_x).x();
		mat_rot_y = Rotation(rot_y).y();
		mat_rot_z = Rotation(rot_z).z();

		mat_translate(4, 1) = center.x;
		mat_translate(4, 2) = center.y;
		mat_translate(4, 3) = center.z;

		return mat_scale * mat_rot_x * mat_rot_y * mat_rot_z * mat_translate * projection;
	}

	Color color_from_conf(ini::Section &conf) {
		auto c = conf["color"].as_double_tuple_or_die();
		double b = c.at(2);
		double g = c.at(1);
		double r = c.at(0);
		return { r, g, b };
	}

	void platonic(ini::Section &conf, Matrix &project, vector<Line3D> &lines, Point3D *points, unsigned int points_len, Edge *edges, unsigned int edges_len) {
		auto mat = transform_from_conf(conf, project);
		auto color = color_from_conf(conf);

		for (auto p = points; p != points + points_len; p++) {
			*p *= mat;
		}
		lines.reserve(lines.size() + edges_len);
		for (auto c = edges; c != edges + edges_len; c++) {
			assert(c->a < points_len);
			assert(c->b < points_len);
			auto a = points[c->a];
			auto b = points[c->b];
			lines.push_back({{a.x, a.y, a.z}, {b.x, b.y, b.z}, color.to_img_color()});
		}
	}

	TriangleFigure platonic(ini::Section &conf, Matrix &project, vector<Point3D> points, vector<Face> faces) {
		auto mat = transform_from_conf(conf, project);
		auto color = color_from_conf(conf);

		for (auto &p : points) {
			p *= mat;
		}

		TriangleFigure fig;
		fig.points = points;
		fig.faces = faces;
		fig.ambient = color;
		return fig;
	}

	struct Frustum {
		double near, far;
		double fov, aspect;
		bool use;
	};

	static void common_conf(
		const ini::Configuration &conf,
		img::Color &background,
		int &size,
		Matrix &mat_eye,
		int &nr_fig,
		Frustum &frustum
	) {
		background = tup_to_color(conf["General"]["backgroundcolor"].as_double_tuple_or_die());
		size = conf["General"]["size"].as_int_or_die();
		auto eye = tup_to_point3d(conf["General"]["eye"].as_double_tuple_or_die());
		nr_fig = conf["General"]["nrFigures"].as_int_or_die();

		auto clipping = conf["General"]["clipping"].as_bool_or_default(false);

		// Create projection matrix
		Vector3D dir;
		if (clipping) {
			dir = tup_to_vector3d(conf["General"]["viewDirection"].as_double_tuple_or_die());
			frustum.use = true;
			frustum.near = conf["General"]["dNear"].as_double_or_die();
			frustum.far = conf["General"]["dFar"].as_double_or_die();
			frustum.fov = deg2rad(conf["General"]["hfov"].as_double_or_die());
			frustum.aspect = conf["General"]["aspectRatio"].as_double_or_die();
		} else {
			frustum.use = false;
			dir = -eye;
		}

		auto r = dir.length();
		auto theta = atan2(-dir.y, -dir.x);
		auto phi = acos(-dir.z / r);

		Matrix mat_tr, mat_rot_z, mat_rot_x;

		mat_tr(4, 1) = -eye.x;
		mat_tr(4, 2) = -eye.y;
		mat_tr(4, 3) = -eye.z;
		mat_rot_z = Rotation(-(theta + M_PI / 2)).z();
		mat_rot_x = Rotation(-phi).x();

		mat_eye = mat_tr * mat_rot_z * mat_rot_x;
	}

	img::EasyImage wireframe(const ini::Configuration &conf, bool with_z) {
		img::Color bg;
		int size, nr_fig;
		Matrix mat_eye;
		Frustum frustum;
		common_conf(conf, bg, size, mat_eye, nr_fig, frustum);

		// Parse figures
		vector<Line3D> lines;
		for (int i = 0; i < nr_fig; i++) {
			auto fig = conf[string("Figure") + to_string(i)];
			auto type = fig["type"].as_string_or_die();
			if (type == "LineDrawing") {
				wireframe::line_drawing(fig, mat_eye, lines);
			} else if (type == "Cube") {
				cube(fig, mat_eye, lines);
			} else if (type == "Tetrahedron") {
				tetrahedron(fig, mat_eye, lines);
			} else if (type == "Octahedron") {
				octahedron(fig, mat_eye, lines);
			} else if (type == "Icosahedron") {
				icosahedron(fig, mat_eye, lines);
			} else if (type == "Dodecahedron") {
				dodecahedron(fig, mat_eye, lines);
			} else if (type == "Cylinder") {
				cylinder(fig, mat_eye, lines);
			} else if (type == "Cone") {
				cone(fig, mat_eye, lines);
			} else if (type == "Sphere") {
				sphere(fig, mat_eye, lines);
			} else if (type == "Torus") {
				torus(fig, mat_eye, lines);
			} else if (type == "BuckyBall") {
				buckyball(fig, mat_eye, lines);
			} else if (type == "3DLSystem") {
				wireframe::l_system(fig, mat_eye, lines);
			} else if (type == "FractalCube") {
				fractal_cube(fig, mat_eye, lines);
			} else if (type == "FractalOctahedron") {
				fractal_octahedron(fig, mat_eye, lines);
			} else if (type == "FractalTetrahedron") {
				fractal_tetrahedron(fig, mat_eye, lines);
			} else if (type == "FractalIcosahedron") {
				fractal_icosahedron(fig, mat_eye, lines);
			} else if (type == "FractalDodecahedron") {
				fractal_dodecahedron(fig, mat_eye, lines);
			} else if (type == "FractalBuckyBall") {
				fractal_buckyball(fig, mat_eye, lines);
			} else if (type == "MengerSponge") {
				puts("TODO MengerSponge lines");
			} else {
				throw TypeException(type);
			}
		}

		// Draw
		return Lines3D(lines).draw(size, bg, with_z);
	}

	/**
	 * \brief Interpolate between two points
	 */
	static Point3D interpolate(Point3D from, Point3D to, double p) {
		// If two points are *very* close to a plane there may be significant imprecision
		// that causes p to far exceed the value it should have.
		// Clamping it works well enough as a workaround.
		p = min(p, 1.0);
		assert(p >= 0 && "p must be between 0 and 1");
		assert(p <= 1 && "p must be between 0 and 1");
		return Point3D(
			from.x * p + to.x * (1 - p),
			from.y * p + to.y * (1 - p),
			from.z * p + to.z * (1 - p)
		);
	}

	/**
	 * \brief Apply frustum clipping.
	 */
	template<typename B, typename P>
	void frustum_apply(TriangleFigure &f, B bitfield, P project) {
		auto proj = [&f, &project](auto from_i, auto to_i) {
			assert(from_i < f.points.size());
			assert(to_i < f.points.size());
			auto from = f.points[from_i];
			auto to = f.points[to_i];
			auto p = interpolate(from, to, project(from, to));
			f.points.push_back(p);
			return (unsigned int)(f.points.size() - 1);
		};
		auto swap_remove = [&f](size_t i) {
			// Swap, then remove. This is always O(1)
			if (i < f.faces.size()) {
				f.faces[i] = f.faces.back();
			}
			f.faces.pop_back();
		};
		for (size_t i = 0; i < f.faces.size(); i++) {
			auto &t = f.faces[i];
			switch(bitfield(t)) {
			// Nothing to do
			case 0b000:
				break;
			// Split triangle
			case 0b100: {
				auto p = proj(t.a, t.b);
				auto q = proj(t.a, t.c);
				t.a = p;
				f.faces.push_back({ p, q, t.c });
				break;
			}
			case 0b010: {
				auto p = proj(t.b, t.c);
				auto q = proj(t.b, t.a);
				t.b = p;
				f.faces.push_back({ p, q, t.a });
				break;
			}
			case 0b001: {
				auto p = proj(t.c, t.a);
				auto q = proj(t.c, t.b);
				t.c = p;
				f.faces.push_back({ p, q, t.b });
				break;
			}
			// Shrink triangle
			case 0b011:
				t.b = proj(t.b, t.a);
				t.c = proj(t.c, t.a);
				break;
			case 0b101:
				t.c = proj(t.c, t.b);
				t.a = proj(t.a, t.b);
				break;
			case 0b110:
				t.a = proj(t.a, t.c);
				t.b = proj(t.b, t.c);
				break;
			// Remove triangle
			case 0b111:
				swap_remove(i--);
				break;
			default:
				UNREACHABLE;
			}
		}
	}

	img::EasyImage triangles(const ini::Configuration &conf) {
		img::Color bg;
		int size, nr_fig;
		Matrix mat_eye;
		Frustum frustum;
		common_conf(conf, bg, size, mat_eye, nr_fig, frustum);

		// Parse figures
		vector<TriangleFigure> figures;
		figures.reserve(nr_fig);
		for (int i = 0; i < nr_fig; i++) {
			TriangleFigure fig;
			auto sect = conf[string("Figure") + to_string(i)];
			auto type = sect["type"].as_string_or_die();
			if (type == "Cube") {
				fig = cube(sect, mat_eye);
			} else if (type == "Tetrahedron") {
				fig = tetrahedron(sect, mat_eye);
			} else if (type == "Octahedron") {
				fig = octahedron(sect, mat_eye);
			} else if (type == "Icosahedron") {
				fig = icosahedron(sect, mat_eye);
			} else if (type == "Dodecahedron") {
				fig = dodecahedron(sect, mat_eye);
			} else if (type == "Cylinder") {
				fig = cylinder(sect, mat_eye);
			} else if (type == "Cone") {
				fig = cone(sect, mat_eye);
			} else if (type == "Sphere") {
				fig = sphere(sect, mat_eye);
			} else if (type == "Torus") {
				fig = torus(sect, mat_eye);
			} else if (type == "BuckyBall") {
				puts("TODO BuckyBall triangles");
				continue;
			} else if (type == "FractalCube") {
				fig = fractal_cube(sect, mat_eye);
			} else if (type == "FractalTetrahedron") {
				fig = fractal_tetrahedron(sect, mat_eye);
			} else if (type == "FractalOctahedron") {
				fig = fractal_octahedron(sect, mat_eye);
			} else if (type == "FractalIcosahedron") {
				fig = fractal_icosahedron(sect, mat_eye);
			} else if (type == "FractalDodecahedron") {
				fig = fractal_dodecahedron(sect, mat_eye);
			} else if (type == "FractalBuckyBall") {
				puts("TODO FractalBuckyBall triangles");
				continue;
			} else if (type == "MengerSponge") {
				puts("TODO MengerSponge triangles");
				continue;
			} else {
				throw TypeException(type);
			}
			figures.push_back(fig);
		}

		// Clipping
		if (frustum.use) {

			// Near & far plane
			// TODO experiment with loop order to see which is faster
			for (auto &f : figures) {
				enum Plane {
					NEAR,
					FAR,
					RIGHT,
					LEFT,
					TOP,
					DOWN,
				};
				auto outside = [&frustum](Point3D p, int plane, double v) {
					switch (plane) {
						case NEAR : return -p.z < frustum.near;
						case FAR  : return -p.z > frustum.far;
						case RIGHT: return p.x * frustum.near / -p.z > v;
						case LEFT : return p.x * frustum.near / -p.z < v;
						case TOP  : return p.y * frustum.near / -p.z > v;
						case DOWN : return p.y * frustum.near / -p.z < v;
						default:
									assert(!"Invalid direction");
									return false;
					}
				};
				auto outside_mask = [&f, &outside](Face &t, int plane, double v) {
					assert(t.a < f.points.size());
					assert(t.b < f.points.size());
					assert(t.c < f.points.size());
					return (int)outside(f.points[t.a], plane, v) << 2
						| (int)outside(f.points[t.b], plane, v) << 1
						| (int)outside(f.points[t.c], plane, v);
				};

				// Near & far
				auto dnear = frustum.near, dfar = frustum.far;
				{
					frustum_apply(f,
						[&outside_mask](auto &t) { return outside_mask(t, NEAR, NAN); },
						[dnear, dfar](Point3D from, Point3D to) {
							return (-dnear - to.z) / (from.z - to.z);
						}
					);
					frustum_apply(f,
						[&outside_mask](auto &t) { return outside_mask(t, FAR, NAN); },
						[dnear, dfar](Point3D from, Point3D to) {
							return (-dfar - to.z) / (from.z - to.z);
						}
					);
				}

				// Left & right plane
				auto right = dnear * tan(frustum.fov / 2);
				{
					frustum_apply(f,
						[&outside_mask, right](auto &t) { return outside_mask(t, RIGHT, right); },
						[dnear, right](Point3D from, Point3D to) {
							return (to.x * dnear + to.z * right) /
								((to.x - from.x) * dnear + (to.z - from.z) * right);
						}
					);
					frustum_apply(f,
						[&outside_mask, right](auto &t) { return outside_mask(t, LEFT, -right); },
						[dnear, right](Point3D from, Point3D to) {
							return (to.x * dnear + to.z * -right) /
								((to.x - from.x) * dnear + (to.z - from.z) * -right);
						}
					);
				}

				// Top & down plane
				auto top = right / frustum.aspect;
				{
					frustum_apply(f,
						[&outside_mask, top](auto &t) { return outside_mask(t, TOP, top); },
						[dnear, top](Point3D from, Point3D to) {
							return (to.y * dnear + to.z * top) /
								((to.y - from.y) * dnear + (to.z - from.z) * top);
						}
					);
					frustum_apply(f,
						[&outside_mask, top](auto &t) { return outside_mask(t, DOWN, -top); },
						[dnear, top](Point3D from, Point3D to) {
							return (to.y * dnear + to.z * -top) /
								((to.y - from.y) * dnear + (to.z - from.z) * -top);
						}
					);
				}
			}
		}

		// Draw
		return draw(figures, size, bg);
	}

	img::EasyImage draw(const std::vector<TriangleFigure> &figures, unsigned int size, img::Color background) {
		// TODO wdym "unitialized", GCC?
		double d = NAN, offset_x = NAN, offset_y = NAN;

		if (figures.empty()) {
			return img::EasyImage(0, 0);
		}

		Point3D a, b, c;
		auto f2p = [&a, &b, &c](auto &f, auto &t) {
			a = f.points[t.a];
			b = f.points[t.b];
			c = f.points[t.c];
		};

		// Determine bounds
		double min_x, max_x, min_y, max_y;
		min_x = min_y = +numeric_limits<double>::infinity();
		max_x = max_y = -numeric_limits<double>::infinity();
		for (auto &f : figures) {
			for (auto &t : f.faces) {
				f2p(f, t);
				assert(a.z != 0 && "division by 0");
				assert(b.z != 0 && "division by 0");
				assert(c.z != 0 && "division by 0");
				min_x = min(min(min_x, a.x / -a.z), min(b.x / -b.z, c.x / -c.z));
				min_y = min(min(min_y, a.y / -a.z), min(b.y / -b.z, c.y / -c.z));
				max_x = max(max(max_x, a.x / -a.z), max(b.x / -b.z, c.x / -c.z));
				max_y = max(max(max_y, a.y / -a.z), max(b.y / -b.z, c.y / -c.z));
			}
		}

		auto img = create_img(min_x, min_y, max_x, max_y, size, background, d, offset_x, offset_y);

		assert(!isnan(d));
		assert(!isnan(offset_x));
		assert(!isnan(offset_y));

#if GRAPHICS_DEBUG > 0
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
#endif

		// Transform & draw triangles
		ZBuffer z(img.get_width(), img.get_height());
		for (auto &f : figures) {
#if GRAPHICS_DEBUG > 0
			size_t color_i = 0;
#endif
			for (auto &t : f.faces) {
				f2p(f, t);
#if GRAPHICS_DEBUG > 0
				auto color = colors_pool[color_i++];
				color_i %= sizeof(colors_pool) / sizeof(*colors_pool);
#else
				auto color = f.ambient.to_img_color();
#endif
				img.draw_zbuf_triag(z, a, b, c, d, offset_x, offset_y, color);
			}
		}

#if GRAPHICS_DEBUG > 0
		for (auto &f : figures) {
			for (auto &t : f.faces) {
				f2p(f, t);
				Point3D la(a.x / -a.z * d + offset_x, a.y / -a.z * d + offset_y, a.z);
				Point3D lb(b.x / -b.z * d + offset_x, b.y / -b.z * d + offset_y, b.z);
				Point3D lc(c.x / -c.z * d + offset_x, c.y / -c.z * d + offset_y, c.z);
				auto color = img::Color(255 - f.ambient.red, 255 - f.ambient.green, 255 - f.ambient.blue);
				Line3D(la, lb, color).draw(img, z);
				Line3D(lb, lc, color).draw(img, z);
				Line3D(lc, la, color).draw(img, z);
			}
		}
#endif

		return img;
	}
}
