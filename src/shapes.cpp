#include "shapes.h"
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

	static void common_conf(const ini::Configuration &conf, Color &background, int &size, Matrix &mat_eye, int &nr_fig) {
		background = tup_to_color(conf["General"]["backgroundcolor"].as_double_tuple_or_die());
		size = conf["General"]["size"].as_int_or_die();
		auto eye = tup_to_point3d(conf["General"]["eye"].as_double_tuple_or_die());
		nr_fig = conf["General"]["nrFigures"].as_int_or_die();

		// Create projection matrix
		auto r = eye.length();
		auto theta = atan2(eye.y, eye.x);
		auto phi = acos(eye.z / r);

		auto theta_sin = sin(theta);
		auto theta_cos = cos(theta);

		auto phi_sin = sin(phi);
		auto phi_cos = cos(phi);

		mat_eye(1, 1) = -theta_sin;
		mat_eye(1, 2) = -theta_cos * phi_cos;
		mat_eye(1, 3) = theta_cos * phi_sin;

		mat_eye(2, 1) = theta_cos;
		mat_eye(2, 2) = -theta_sin * phi_cos;
		mat_eye(2, 3) = theta_sin * phi_sin;

		mat_eye(3, 2) = phi_sin;
		mat_eye(3, 3) = phi_cos;

		mat_eye(4, 3) = -r;
	}

	img::EasyImage wireframe(const ini::Configuration &conf, bool with_z) {
		Color bg;
		int size, nr_fig;
		Matrix mat_eye;
		common_conf(conf, bg, size, mat_eye, nr_fig);

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
		common_conf(conf, bg, size, mat_eye, nr_fig);

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

		// Draw
		return Triangles3D(triangles).draw(size, bg);
	}
}
