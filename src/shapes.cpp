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
	Matrix transform_from_conf(const ini::Section &conf, const Matrix &projection) {
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

	static Color color_from_conf(const vector<double> &c) {
		double b = c.at(2);
		double g = c.at(1);
		double r = c.at(0);
		return { r, g, b };
	}

	static Color color_from_conf(const ini::Entry &e) {
		return color_from_conf(e.as_double_tuple_or_die());
	}

	static Color try_color_from_conf(const ini::Entry &e) {
		vector<double> c;
		if (e.as_double_tuple_if_exists(c)) {
			return color_from_conf(c);
		}
		return Color();
	}

	Color color_from_conf(const ini::Section &conf) {
		return color_from_conf(conf["color"]);
	}

	vector<Vector3D> calculate_face_normals(const vector<Point3D> &points, const vector<Face> &faces) {
		vector<Vector3D> normals;
		normals.reserve(faces.size());
		for (auto &f : faces) {
			assert(f.a < points.size());
			assert(f.b < points.size());
			assert(f.c < points.size());
			auto a = points[f.a];
			auto b = points[f.b];
			auto c = points[f.c];
			auto n = (b - a).cross(c - a);
			n.normalise();
			normals.push_back(n);
		}
		return normals;
	}

	void platonic(const FigureConfiguration &conf, vector<Line3D> &lines, Point3D *points, unsigned int points_len, Edge *edges, unsigned int edges_len) {
		auto mat = transform_from_conf(conf.section, conf.eye);
		auto color = color_from_conf(conf.section);

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

	TriangleFigure platonic(const FigureConfiguration &conf, vector<Point3D> points, vector<Face> faces) {

		TriangleFigure fig;
		fig.points = points;
		fig.faces = faces;
		if (conf.with_lighting) {
			assert(conf.face_normals && "TODO: vertex normals");
			fig.normals = calculate_face_normals(points, faces);
		}
		fig.face_normals = conf.face_normals;

		auto mat = transform_from_conf(conf.section, conf.eye);
		if (conf.with_lighting) {
			fig.ambient = try_color_from_conf(conf.section["ambientReflection"]);
			fig.diffuse = try_color_from_conf(conf.section["diffuseReflection"]);
			fig.specular = try_color_from_conf(conf.section["specularReflection"]);
		} else {
			fig.ambient = color_from_conf(conf.section);
		}

		for (auto &p : fig.points) {
			p *= mat;
		}
		for (auto &n : fig.normals) {
			n *= mat;
		}

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
			auto section = conf[string("Figure") + to_string(i)];
			FigureConfiguration fconf {
				section,
				mat_eye,
				false,
				false,
			};
			auto type = fconf.section["type"].as_string_or_die();
			if (type == "LineDrawing") {
				wireframe::line_drawing(fconf.section, fconf.eye, lines);
			} else if (type == "Cube") {
				cube(fconf, lines);
			} else if (type == "Tetrahedron") {
				tetrahedron(fconf, lines);
			} else if (type == "Octahedron") {
				octahedron(fconf, lines);
			} else if (type == "Icosahedron") {
				icosahedron(fconf, lines);
			} else if (type == "Dodecahedron") {
				dodecahedron(fconf, lines);
			} else if (type == "Cylinder") {
				cylinder(fconf, lines);
			} else if (type == "Cone") {
				cone(fconf, lines);
			} else if (type == "Sphere") {
				sphere(fconf, lines);
			} else if (type == "Torus") {
				torus(fconf, lines);
			} else if (type == "BuckyBall") {
				buckyball(fconf, lines);
			} else if (type == "3DLSystem") {
				wireframe::l_system(fconf.section, fconf.eye, lines);
			} else if (type == "FractalCube") {
				fractal_cube(fconf, lines);
			} else if (type == "FractalOctahedron") {
				fractal_octahedron(fconf, lines);
			} else if (type == "FractalTetrahedron") {
				fractal_tetrahedron(fconf, lines);
			} else if (type == "FractalIcosahedron") {
				fractal_icosahedron(fconf, lines);
			} else if (type == "FractalDodecahedron") {
				fractal_dodecahedron(fconf, lines);
			} else if (type == "FractalBuckyBall") {
				fractal_buckyball(fconf, lines);
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

	img::EasyImage triangles(const ini::Configuration &conf, bool with_lighting) {
		img::Color bg;
		int size, nr_fig;
		Matrix mat_eye;
		Frustum frustum;
		Lights lights;

		common_conf(conf, bg, size, mat_eye, nr_fig, frustum);

#if GRAPHICS_DEBUG > 0
		lights.eye = mat_eye;
#endif

		// Parse figures
		vector<TriangleFigure> figures;
		figures.reserve(nr_fig);
		for (int i = 0; i < nr_fig; i++) {
			auto section = conf[string("Figure") + to_string(i)];
			FigureConfiguration fconf {
				section,
				mat_eye,
				with_lighting,
				true,
			};
			auto type = fconf.section["type"].as_string_or_die();
			TriangleFigure fig;
			if (type == "Cube") {
				fig = cube(fconf);
			} else if (type == "Tetrahedron") {
				fig = tetrahedron(fconf);
			} else if (type == "Octahedron") {
				fig = octahedron(fconf);
			} else if (type == "Icosahedron") {
				fig = icosahedron(fconf);
			} else if (type == "Dodecahedron") {
				fig = dodecahedron(fconf);
			} else if (type == "Cylinder") {
				fig = cylinder(fconf);
			} else if (type == "Cone") {
				fig = cone(fconf);
			} else if (type == "Sphere") {
				fig = sphere(fconf);
			} else if (type == "Torus") {
				fig = torus(fconf);
			} else if (type == "BuckyBall") {
				puts("TODO BuckyBall triangles");
				continue;
			} else if (type == "FractalCube") {
				fig = fractal_cube(fconf);
			} else if (type == "FractalTetrahedron") {
				fig = fractal_tetrahedron(fconf);
			} else if (type == "FractalOctahedron") {
				fig = fractal_octahedron(fconf);
			} else if (type == "FractalIcosahedron") {
				fig = fractal_icosahedron(fconf);
			} else if (type == "FractalDodecahedron") {
				fig = fractal_dodecahedron(fconf);
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

		if (with_lighting) {
			int nr_light = conf["General"]["nrLights"];
			for (int i = 0; i < nr_light; i++) {
				auto section = conf[string("Light") + to_string(i)];
				vector<double> color;
				lights.ambient += color_from_conf(section["ambientLight"]);
				if (section["infinity"].as_bool_or_default(false)) {
					auto d = tup_to_vector3d(section["direction"].as_double_tuple_or_die());
					d.normalise();
					d *= mat_eye;
					lights.directional.push_back({
						d,
						color_from_conf(section["diffuseLight"]),
					});
				}
			}
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
		return draw(figures, lights, size, bg);
	}

	img::EasyImage draw(const std::vector<TriangleFigure> &figures, const Lights &lights, unsigned int size, img::Color background) {
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

		// Transform & draw triangles
		ZBuffer z(img.get_width(), img.get_height());
		for (auto &f : figures) {
			// Apply ambient light
			auto ambient_color = f.ambient * lights.ambient;
#if GRAPHICS_DEBUG_FACES > 0
			size_t color_i = 0;
#endif
			for (size_t i = 0; i < f.faces.size(); i++) {
				auto color = ambient_color;
				auto &t = f.faces[i];
				f2p(f, t);
#if GRAPHICS_DEBUG_FACES > 0
				auto clr = colors_pool[color_i++];
				color_i %= color_pool_size;
#else
				// Apply lights
				for (auto &d : lights.directional) {
					assert(f.faces.size() == f.normals.size() && "No normals");
					color += (f.diffuse * d.diffuse) * max(f.normals[i].dot(-d.direction), 0.0);
				}
				assert(color.r >= 0 && "Colors can't be negative");
				assert(color.g >= 0 && "Colors can't be negative");
				assert(color.b >= 0 && "Colors can't be negative");
				auto clr = color.to_img_color();
#endif
				img.draw_zbuf_triag(z, a, b, c, d, offset_x, offset_y, clr);
			}
		}

#if GRAPHICS_DEBUG_NORMALS > 0
		for (auto &f : figures) {
			size_t color_i = 3; // Take an opposite color for clarity
			if (f.face_normals) {
				for (size_t i = 0; i < f.faces.size(); i++) {
					auto &t = f.faces[i];
					f2p(f, t);
					auto from = Point3D::center({ a, b, c });
					auto to = from + f.normals[i] * 0.1;
					Point3D ft(from.x / -from.z * d + offset_x, from.y / -from.z * d + offset_y, from.z);
					Point3D tt(to.x / -to.z * d + offset_x, to.y / -to.z * d + offset_y, to.z);
					auto clr = colors_pool[color_i++];
					color_i %= color_pool_size;
					Line3D(ft, tt, clr).draw_clip(img, z);
				}
			} else {
				assert(!"TODO vertex normals");
			}
		}
#endif

#if GRAPHICS_DEBUG > 1
		for (auto &f : figures) {
			for (auto &t : f.faces) {
				f2p(f, t);
				Point3D la(a.x / -a.z * d + offset_x, a.y / -a.z * d + offset_y, a.z);
				Point3D lb(b.x / -b.z * d + offset_x, b.y / -b.z * d + offset_y, b.z);
				Point3D lc(c.x / -c.z * d + offset_x, c.y / -c.z * d + offset_y, c.z);
				auto clr = f.ambient.to_img_color();
				clr = img::Color(255 - clr.red, 255 - clr.green, 255 - clr.blue);
				Line3D(la, lb, clr).draw(img, z);
				Line3D(lb, lc, clr).draw(img, z);
				Line3D(lc, la, clr).draw(img, z);
			}
		}
#endif

#if GRAPHICS_DEBUG > 0
		// Axes
		{
			Point3D o;
			o *= lights.eye;
			auto ox = o + Vector3D::vector(1000, 0, 0) * lights.eye;
			auto oy = o + Vector3D::vector(0, 1000, 0) * lights.eye;
			auto oz = o + Vector3D::vector(0, 0, 1000) * lights.eye;
			Point3D lo(o.x / -o.z * d + offset_x, o.y / -o.z * d + offset_y, o.z);
			Point3D lox(ox.x / -ox.z * d + offset_x, ox.y / -ox.z * d + offset_y, ox.z);
			Point3D loy(oy.x / -oy.z * d + offset_x, oy.y / -oy.z * d + offset_y, oy.z);
			Point3D loz(oz.x / -oz.z * d + offset_x, oz.y / -oz.z * d + offset_y, oz.z);
			Line3D(lo, lox, img::Color(255, 0, 0)).draw_clip(img, z);
			Line3D(lo, loy, img::Color(0, 255, 0)).draw_clip(img, z);
			Line3D(lo, loz, img::Color(0, 0, 255)).draw_clip(img, z);
		}
#endif

		return img;
	}
}
