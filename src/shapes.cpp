#include "shapes.h"
#include <initializer_list>
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

	img::Color color_from_conf(ini::Section &conf) {
		return tup_to_color(conf["color"].as_double_tuple_or_die());
	}

	void platonic(ini::Section &conf, Matrix &project, vector<Line3D> &lines, Vector3D *points, unsigned int points_len, Edge *edges, unsigned int edges_len) {
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
			lines.push_back({{a.x, a.y, a.z}, {b.x, b.y, b.z}, color});
		}
	}

	void platonic(ini::Section &conf, Matrix &project, vector<Triangle3D> &triangles, Vector3D *points, unsigned int points_len, Face *faces, unsigned int faces_len) {
		auto mat = transform_from_conf(conf, project);
		auto color = color_from_conf(conf);

		for (auto p = points; p != points + points_len; p++) {
			*p *= mat;
		}
		triangles.reserve(triangles.size() + faces_len);
		for (auto f = faces; f != faces + faces_len; f++) {
			assert(f->a < points_len);
			assert(f->b < points_len);
			assert(f->c < points_len);
			auto a = points[f->a];
			auto b = points[f->b];
			auto c = points[f->c];
			triangles.push_back({{a.x, a.y, a.z}, {b.x, b.y, b.z}, {c.x, c.y, c.z}, color});
		}
	}

	struct Frustum {
		double near, far;
		double fov, aspect;
		bool use;
	};

	static void common_conf(
		const ini::Configuration &conf,
		Color &background,
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
		Color bg;
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

	img::EasyImage triangles(const ini::Configuration &conf) {
		Color bg;
		int size, nr_fig;
		Matrix mat_eye;
		Frustum frustum;
		common_conf(conf, bg, size, mat_eye, nr_fig, frustum);

		// Parse figures
		vector<Triangle3D> triangles;
		for (int i = 0; i < nr_fig; i++) {
			auto fig = conf[string("Figure") + to_string(i)];
			auto type = fig["type"].as_string_or_die();
			if (type == "Cube") {
				cube(fig, mat_eye, triangles);
			} else if (type == "Tetrahedron") {
				tetrahedron(fig, mat_eye, triangles);
			} else if (type == "Octahedron") {
				octahedron(fig, mat_eye, triangles);
			} else if (type == "Icosahedron") {
				icosahedron(fig, mat_eye, triangles);
			} else if (type == "Dodecahedron") {
				dodecahedron(fig, mat_eye, triangles);
			} else if (type == "Cylinder") {
				cylinder(fig, mat_eye, triangles);
			} else if (type == "Cone") {
				cone(fig, mat_eye, triangles);
			} else if (type == "Sphere") {
				sphere(fig, mat_eye, triangles);
			} else if (type == "Torus") {
				torus(fig, mat_eye, triangles);
			} else if (type == "BuckyBall") {
				puts("TODO BuckyBall triangles");
			} else if (type == "FractalCube") {
				fractal_cube(fig, mat_eye, triangles);
			} else if (type == "FractalTetrahedron") {
				fractal_tetrahedron(fig, mat_eye, triangles);
			} else if (type == "FractalOctahedron") {
				fractal_octahedron(fig, mat_eye, triangles);
			} else if (type == "FractalIcosahedron") {
				fractal_icosahedron(fig, mat_eye, triangles);
			} else if (type == "FractalDodecahedron") {
				fractal_dodecahedron(fig, mat_eye, triangles);
			} else if (type == "FractalBuckyBall") {
				puts("TODO FractalBuckyBall triangles");
			} else if (type == "MengerSponge") {
				puts("TODO MengerSponge triangles");
			} else {
				throw TypeException(type);
			}
		}

#ifdef GRAPHICS_DEBUG
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
		size_t color_i = 0;
		for (auto &t : triangles) {
			t.color = colors_pool[color_i++];
			color_i %= sizeof(colors_pool) / sizeof(*colors_pool);
		}
#endif

		// Clipping
		if (frustum.use) {
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
				case LEFT : return p.x * frustum.near / -p.z < -v;
				case TOP  : return p.y * frustum.near / -p.z > v;
				case DOWN : return p.y * frustum.near / -p.z < -v;
				default:
					assert(!"Invalid direction");
					return false;
				}
			};
			auto outside_mask = [&outside](Triangle3D &t, int plane, double v) {
				return (int)outside(t.a, plane, v) << 2
					| (int)outside(t.b, plane, v) << 1
					| (int)outside(t.c, plane, v);
			};

			auto swap_remove = [&triangles](size_t i) {
				// Swap, then remove. This is always O(1)
				if (i < triangles.size()) {
					triangles[i] = triangles.back();
				}
				triangles.pop_back();
			};
			auto interpolate = [](Point3D from, Point3D to, double p) {
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
			};

			// Near & far plane
			for (auto plane : { NEAR, FAR }) {
				for (size_t i = 0; i < triangles.size(); i++) {
					auto project = [plane, &frustum, interpolate](Point3D from, Point3D to) {
						auto dval = plane == NEAR ? -frustum.near : -frustum.far;
						return interpolate(from, to, (dval - to.z) / (from.z - to.z));
					};

					auto &t = triangles[i];
					switch(outside_mask(t, plane, NAN)) {
					// Nothing to do
					case 0b000:
						break;
					// Split triangle
					case 0b100: {
						auto p = project(t.a, t.b);
						auto q = project(t.a, t.c);
						t.a = p;
						triangles.push_back({ p, q, t.c, t.color });
						break;
					}
					case 0b010: {
						auto p = project(t.b, t.c);
						auto q = project(t.b, t.a);
						t.b = p;
						triangles.push_back({ p, q, t.a, t.color });
						break;
					}
					case 0b001: {
						auto p = project(t.c, t.a);
						auto q = project(t.c, t.b);
						t.c = p;
						triangles.push_back({ p, q, t.b, t.color });
						break;
					}
					// Shrink triangle
					case 0b011:
						t.b = project(t.b, t.a);
						t.c = project(t.c, t.a);
						break;
					case 0b101:
						t.c = project(t.c, t.b);
						t.a = project(t.a, t.b);
						break;
					case 0b110:
						t.a = project(t.a, t.c);
						t.b = project(t.b, t.c);
						break;
					// Remove triangle
					case 0b111:
						swap_remove(i--);
						break;
					default:
						assert(!"unreachable");
					}
				}
			}

			// Left & right plane
			auto right = frustum.near * tan(frustum.fov / 2);
			for (auto plane : { RIGHT, LEFT }) {
				for (size_t i = 0; i < triangles.size(); i++) {
					auto project = [plane, &frustum, interpolate, right](Point3D from, Point3D to) {
						auto dval = plane == RIGHT ? right : -right;
						auto p = (to.x * frustum.near + to.z * dval) /
							((to.x - from.x) * frustum.near + (to.z - from.z) * dval);
						return interpolate(from, to, p);
					};

					auto &t = triangles[i];
					switch(outside_mask(t, plane, right)) {
					// Nothing to do
					case 0b000:
						break;
					// Split triangle
					case 0b100: {
						auto p = project(t.a, t.b);
						auto q = project(t.a, t.c);
						t.a = p;
						triangles.push_back({ p, q, t.c, t.color });
						break;
					}
					case 0b010: {
						auto p = project(t.b, t.c);
						auto q = project(t.b, t.a);
						t.b = p;
						triangles.push_back({ p, q, t.a, t.color });
						break;
					}
					case 0b001: {
						auto p = project(t.c, t.a);
						auto q = project(t.c, t.b);
						t.c = p;
						triangles.push_back({ p, q, t.b, t.color });
						break;
					}
					// Shrink triangle
					case 0b011:
						t.b = project(t.b, t.a);
						t.c = project(t.c, t.a);
						break;
					case 0b101:
						t.c = project(t.c, t.b);
						t.a = project(t.a, t.b);
						break;
					case 0b110:
						t.a = project(t.a, t.c);
						t.b = project(t.b, t.c);
						break;
					// Remove triangle
					case 0b111:
						swap_remove(i--);
						break;
					default:
						assert(!"unreachable");
					}
				}
			}

			// Top & down plane
			auto top = right / frustum.aspect;
			for (auto plane : { TOP, DOWN }) {
				for (size_t i = 0; i < triangles.size(); i++) {
					auto project = [plane, &frustum, interpolate, top](Point3D from, Point3D to) {
						auto dval = plane == TOP ? top : -top;
						auto p = (to.y * frustum.near + to.z * dval) /
							((to.y - from.y) * frustum.near + (to.z - from.z) * dval);
						return interpolate(from, to, p);
					};

					auto &t = triangles[i];
					switch(outside_mask(t, plane, top)) {
					// Nothing to do
					case 0b000:
						break;
					// Split triangle
					case 0b100: {
						auto p = project(t.a, t.b);
						auto q = project(t.a, t.c);
						t.a = p;
						triangles.push_back({ p, q, t.c, t.color });
						break;
					}
					case 0b010: {
						auto p = project(t.b, t.c);
						auto q = project(t.b, t.a);
						t.b = p;
						triangles.push_back({ p, q, t.a, t.color });
						break;
					}
					case 0b001: {
						auto p = project(t.c, t.a);
						auto q = project(t.c, t.b);
						t.c = p;
						triangles.push_back({ p, q, t.b, t.color });
						break;
					}
					// Shrink triangle
					case 0b011:
						t.b = project(t.b, t.a);
						t.c = project(t.c, t.a);
						break;
					case 0b101:
						t.c = project(t.c, t.b);
						t.a = project(t.a, t.b);
						break;
					case 0b110:
						t.a = project(t.a, t.c);
						t.b = project(t.b, t.c);
						break;
					// Remove triangle
					case 0b111:
						swap_remove(i--);
						break;
					default:
						assert(!"unreachable");
					}
				}
			}
		}

		// Draw
		return Triangles3D(triangles).draw(size, bg);
	}
}
