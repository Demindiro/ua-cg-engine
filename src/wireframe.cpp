#include "wireframe.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include "engine.h"
#include "easy_image.h"
#include "ini_configuration.h"
#include "l_system.h"
#include "l_parser.h"
#include "lines.h"
#include "vector3d.h"

using namespace std;

namespace wireframe {

	static Matrix transform_from_conf(ini::Section &conf, Matrix &projection) {
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
	
	static img::Color color_from_conf(ini::Section &conf) {
		return tup_to_color(conf["color"].as_double_tuple_or_die());
	}

	static Vector3D polar_to_cartesian(double theta, double phi, double r) {
		auto r_sin_phi = r * sin(phi);
		return Vector3D::point(r_sin_phi * cos(theta), r_sin_phi * sin(theta), r * cos(phi));
	}

	static void line_drawing(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		// Read transformation & color
		auto mat = transform_from_conf(conf, mat_project);
		auto color = color_from_conf(conf);

		// Read points & transform
		vector<Vector3D> points;
		auto points_n = conf["nrPoints"].as_int_or_die();
		points.reserve(points_n);
		for (int i = 0; i < points_n; i++) {
			auto p = tup_to_point3d(conf[string("point") + to_string(i)].as_double_tuple_or_die());
			points.push_back(p * mat);
		}

		// Read lines
		auto lines_n = conf["nrLines"].as_int_or_die();
		lines.reserve(lines.size() + lines_n);
		for (int i = 0; i < lines_n; i++) {
			auto line = conf[string("line") + to_string(i)].as_int_tuple_or_die();
			auto b = points.at(line.at(1)), a = points.at(line.at(0));
			lines.push_back({{a.x, a.y, a.z}, {b.x, b.y, b.z}, color});
		}
	}

	struct Edge {
		int a, b;

		bool operator<(const Edge &rhs) const {
			auto ll = min(a, b), lh = max(a, b);
			auto rl = min(rhs.a, rhs.b), rh = max(rhs.a, rhs.b);
			return ll == rl ? lh < rh : ll < rl;
		}
	};

	struct Face {
		int a, b, c;
	};

	static void platonic(ini::Section &conf, Matrix &project, vector<Line3D> &lines, Vector3D *points, int points_len, Edge *edges, int edges_len) {
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

	static void platonic(ini::Section &conf, Matrix &project, vector<Triangle3D> &triangles, Vector3D *points, int points_len, Face *faces, int faces_len) {
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
	
	static void cube(Vector3D points[8]) {
		points[0] = Vector3D::point( 1,  1,  1);
		points[1] = Vector3D::point( 1,  1, -1);
		points[2] = Vector3D::point( 1, -1,  1);
		points[3] = Vector3D::point( 1, -1, -1);
		points[4] = Vector3D::point(-1,  1,  1);
		points[5] = Vector3D::point(-1,  1, -1);
		points[6] = Vector3D::point(-1, -1,  1);
		points[7] = Vector3D::point(-1, -1, -1);
	}

	static void cube(Edge edges[12]) {
		// X
		edges[0] = { 0, 4 };
		edges[1] = { 1, 5 };
		edges[2] = { 2, 6 };
		edges[3] = { 3, 7 };
		// Y
		edges[4] = { 0, 2 };
		edges[5] = { 1, 3 };
		edges[6] = { 4, 6 };
		edges[7] = { 5, 7 };
		// Z
		edges[ 8] = { 0, 1 };
		edges[ 9] = { 2, 3 };
		edges[10] = { 4, 5 };
		edges[11] = { 6, 7 };
	}

	static void cube(Face faces[12]) {
		// TODO fix order of vertices so culling works properly
		// X
		faces[0] = { 0, 1, 2 };
		faces[1] = { 3, 1, 2 };
		faces[2] = { 4, 5, 6 };
		faces[3] = { 7, 5, 6 };
		// Y
		faces[4] = { 0, 1, 4 };
		faces[5] = { 5, 1, 4 };
		faces[6] = { 2, 3, 6 };
		faces[7] = { 7, 3, 6 };
		// Z
		faces[ 8] = { 0, 2, 4 };
		faces[ 9] = { 2, 6, 4 };
		faces[10] = { 1, 3, 5 };
		faces[11] = { 3, 7, 5 };
	}

	static void cube(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[8];
		Edge edges[12];
		cube(points);
		cube(edges);
		platonic(conf, mat_project, lines, points, 8, edges, 12);
	}

	static void cube(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		Vector3D points[8];
		Face faces[12];
		cube(points);
		cube(faces);
		platonic(conf, mat_project, triangles, points, 8, faces, 12);
	}

	static void tetrahedron(Vector3D points[4]) {
		points[0] = Vector3D::point( 1, -1, -1);
		points[1] = Vector3D::point(-1,  1, -1);
		points[2] = Vector3D::point( 1,  1,  1);
		points[3] = Vector3D::point(-1, -1,  1);
	}

	static void tetrahedron(Edge edges[6]) {
		edges[0] = { 0, 1 };
		edges[1] = { 0, 2 };
		edges[2] = { 0, 3 };
		edges[3] = { 1, 2 };
		edges[4] = { 1, 3 };
		edges[5] = { 2, 3 };
	}

	static void tetrahedron(Face faces[4]) {
		faces[0] = { 0, 1, 2 };
		faces[1] = { 0, 1, 3 };
		faces[2] = { 0, 2, 3 };
		faces[3] = { 1, 2, 3 };
	}

	static void tetrahedron(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[4];
		Edge edges[6];
		tetrahedron(points);
		tetrahedron(edges);
		platonic(conf, mat_project, lines, points, 4, edges, 6);
	}

	static void tetrahedron(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		Vector3D points[4];
		Face faces[6];
		tetrahedron(points);
		tetrahedron(faces);
		platonic(conf, mat_project, triangles, points, 4, faces, 4);
	}

	static void octahedron(Vector3D points[6]) {
		points[0] = Vector3D::point( 1,  0,  0);
		points[1] = Vector3D::point(-1,  0,  0);
		points[2] = Vector3D::point( 0,  1,  0);
		points[3] = Vector3D::point( 0,  0,  1);
		points[4] = Vector3D::point( 0, -1,  0);
		points[5] = Vector3D::point( 0,  0, -1);
	}

	static void octahedron(Edge edges[12]) {
		// Top
		edges[0] = { 0, 2 };
		edges[1] = { 0, 3 };
		edges[2] = { 0, 4 };
		edges[3] = { 0, 5 };
		// Bottom
		edges[4] = { 1, 2 };
		edges[5] = { 1, 3 };
		edges[6] = { 1, 4 };
		edges[7] = { 1, 5 };
		// Ring
		edges[ 8] = { 2, 3 };
		edges[ 9] = { 3, 4 };
		edges[10] = { 4, 5 };
		edges[11] = { 5, 2 };
	}

	static void octahedron(Face faces[8]) {
		// Top
		faces[0] = { 0, 2, 3 };
		faces[1] = { 0, 3, 4 };
		faces[2] = { 0, 4, 5 };
		faces[3] = { 0, 5, 2 };
		// Bottom
		faces[4] = { 1, 2, 3 };
		faces[5] = { 1, 3, 4 };
		faces[6] = { 1, 4, 5 };
		faces[7] = { 1, 5, 2 };
	}

	static void octahedron(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[6];
		Edge edges[12];
		octahedron(points);
		octahedron(edges);
		platonic(conf, mat_project, lines, points, 6, edges, 12);
	}

	static void octahedron(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &lines) {
		Vector3D points[6];
		Face faces[8];
		octahedron(points);
		octahedron(faces);
		platonic(conf, mat_project, lines, points, 6, faces, 8);
	}

	static void icosahedron_points(Vector3D points[12]) {
		auto p = sqrtl(5) / 2;
		points[0] = Vector3D::point(0, 0,  p);
		points[1] = Vector3D::point(0, 0, -p);
		for (int i = 0; i < 5; i++) {
			auto a = (i - 2) * 2 * M_PI / 5;
			auto b = a - M_PI / 5;
			points[2 + i] = Vector3D::point(cos(a), sin(a),  0.5);
			points[7 + i] = Vector3D::point(cos(b), sin(b), -0.5);
		}
		for (int i = 0; i < 12; i++) {
			// Normalize
			points[i] /= p;
		}
	}

	static void icosahedron_edges(Edge edges[30]) {
		for (int i = 0; i < 5; i++) {
			// Top & bottom "hat"
			edges[0 + i] = { 0, 2 + i };
			edges[5 + i] = { 1, 7 + i };
			// Top & bottom ring
			edges[10 + i] = { 2 + i, 2 + (i + 1) % 5 };
			edges[15 + i] = { 7 + i, 7 + (i + 1) % 5 };
			// Middle ring
			edges[20 + i] = { 2 + i, 7 + (i + 0) % 5 };
			edges[25 + i] = { 2 + i, 7 + (i + 1) % 5 };
		}
	}

	static void icosahedron_faces(Face faces[20]) {
		for (int i = 0; i < 5; i++) {
			// TODO double check order of vertices
			int j = (i + 1) % 5;
			// Top & bottom "hat"
			faces[0 + i] = { 0, 2 + i, 2 + j };
			faces[5 + i] = { 1, 7 + i, 7 + j };
			// Ring
			faces[10 + i] = { 2 + i, 7 + i, 7 + j };
			faces[15 + i] = { 2 + i, 2 + j, 7 + j };
		}
	}

	static void icosahedron(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[12];
		Edge edges[30];
		icosahedron_points(points);
		icosahedron_edges(edges);
		platonic(conf, mat_project, lines, points, 12, edges, 30);
	}

	static void dodecahedron(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[20];
		Edge edges[30];
		Vector3D ico[12];
		icosahedron_points(ico);
		for (int i = 0; i < 5; i++) {
			int j = (i + 1) % 5;

			// Top & bottom "hat"
			points[0 + i] = (ico[0] + ico[2 + i] + ico[2 + j]) / 3;
			points[5 + i] = (ico[1] + ico[7 + i] + ico[7 + j]) / 3;
			// Ring
			points[10 + i] = (ico[2 + i] + ico[7 + i] + ico[7 + j]) / 3;
			points[15 + i] = (ico[2 + i] + ico[2 + j] + ico[7 + j]) / 3;

			// Top & bottom ring
			edges[0 + i] = { 0 + i, 0 + j };
			edges[5 + i] = { 5 + i, 5 + j };
			// Top & bottom "surface" ring
			edges[10 + i] = { 0 + i, 15 + i };
			edges[15 + i] = { 5 + i, 10 + i };
			// Middle ring
			edges[20 + i] = { 10 + i, 15 + i };
			edges[25 + i] = { 10 + j, 15 + i };
		}
		platonic(conf, mat_project, lines, points, 20, edges, 30);
	}

	static void circle(vector<Vector3D> &points, int n, double z) {
		double d = 2 * M_PI / n;
		for (int i = 0; i < n; i++) {
			auto x = sin(i * d);
			auto y = cos(i * d);
			points.push_back(Vector3D::point(x, y, z));
		}
	}

	static void cylinder(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		auto n = conf["n"].as_int_or_die();
		auto height = conf["height"].as_double_or_die();

		vector<Vector3D> points;
		points.reserve(n * 2);
		circle(points, n, 0);
		circle(points, n, height);

		vector<Edge> edges(n * 3);
		for (int i = 0; i < n; i++) {
			edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
			edges[1 * n + i] = { 1 * n + i, 1 * n + (i + 1) % n };
			edges[2 * n + i] = { 0 * n + i, 1 * n + i };
		}

		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	static void cone(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		auto n = conf["n"].as_int_or_die();
		auto height = conf["height"].as_double_or_die();

		vector<Vector3D> points;
		points.reserve(n + 1);
		circle(points, n, 0);
		points.push_back(Vector3D::point(0, 0, height));

		vector<Edge> edges(n * 2);
		for (int i = 0; i < n; i++) {
			edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
			edges[1 * n + i] = { 0 * n + i, n };
		}

		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	static void bisect(vector<Vector3D> &points, vector<Edge> &edges, vector<Face> &faces) {
		vector<Edge> new_edges;
		vector<Face> new_faces;
		new_edges.reserve(edges.size() * 2);
		new_faces.reserve(faces.size() * 4);

		map<Edge, int> edge_to_new_point;

		// Use edges to add points & determine new edges
		for (auto e : edges) {
			int i = points.size();
			points.push_back((points[e.a] + points[e.b]) / 2);
			new_edges.push_back({e.a, i});
			new_edges.push_back({i, e.b});
			edge_to_new_point[e] = i;
		}

		// Use faces to create new faces
		for (auto g : faces) {
			Edge ab { g.a, g.b }, bc { g.b, g.c }, ca { g.c, g.a };
			auto d = edge_to_new_point.at(ab);
			auto e = edge_to_new_point.at(bc);
			auto f = edge_to_new_point.at(ca);
			// TODO double check order of vertices
			new_faces.push_back({  d,   e,   f});
			new_faces.push_back({g.a,   d,   f});
			new_faces.push_back({  d, g.b,   e});
			new_faces.push_back({  e,   f, g.c});
			new_edges.push_back({ d, e });
			new_edges.push_back({ e, f });
			new_edges.push_back({ f, d });
		}

		edges = new_edges;
		faces = new_faces;
	}

	static void sphere(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		auto n = conf["n"].as_int_or_die();
		vector<Vector3D> points(12);
		vector<Edge> edges(30);
		vector<Face> faces(20);

		icosahedron_points(points.data());
		icosahedron_edges(edges.data());
		icosahedron_faces(faces.data());

		for (int i = 0; i < n; i++) {
			bisect(points, edges, faces);
		}

		for (auto &p : points) {
			p.normalise();
		}

		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	static void torus(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		auto mr = conf["R"].as_double_or_die();
		auto sr = conf["r"].as_double_or_die();
		auto n = conf["n"].as_int_or_die();
		auto m = conf["m"].as_int_or_die();

		vector<Vector3D> points(m * n);

		double d = 2 * M_PI / m;
		for (int i = 0; i < m; i++) {
			auto x = 0;
			auto y = mr + sin(i * d) * sr;
			auto z = cos(i * d) * sr;
			points[i] = Vector3D::point(x, y, z);
		}

		auto rot = Rotation(2 * M_PI / n).z();
		for (int i = 1; i < n; i++) {
			for (int j = 0; j < m; j++) {
				points[i * m + j] = points[(i - 1) * m + j] * rot;
			}
		}

		vector<Edge> edges;

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < m; j++) {
				edges.push_back({ i * m + j, i * m + (j + 1) % m });
				edges.push_back({ i * m + j, (i + 1) % n * m + j });
			}
		}

		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	struct Cursor3D {
		Vector3D pos;
		Matrix rot;
	};

	struct DrawSystem3D {
		LParser::LSystem3D sys;
		Matrix project;
		Rotation drot;
		Cursor3D current;
		Cursor3D *saved;
		vector<Line3D> &lines;
		mt19937 rng;
		img::Color color;

		DrawSystem3D(vector<Line3D> &lines) : lines(lines) {
			random_device rd;
			rng = mt19937(rd());
		}
	};

	static void draw_sys(DrawSystem3D &s, const string &str, int depth) {
		for (char c : str) {
			auto f = [&](auto rot) { s.current.rot = rot * s.current.rot; };
			switch (c) {
			case '+':
				f(s.drot.z());
				break;
			case '-':
				f(s.drot.inv().z());
				break;
			case '^':
				f(s.drot.inv().y());
				break;
			case '&':
				f(s.drot.y());
				break;
			case '/':
				f(s.drot.x());
				break;
			case '\\':
				f(s.drot.inv().x());
				break;
			case '(':
				*s.saved++ = s.current;
				break;
			case ')':
				s.current = *--s.saved;
				break;
			case '|':
				f(Rotation(-1, 0).z());
				break;
			default:
				if (depth > 0) {
					auto r = s.sys.get_replacement(c);
					auto sum = r.get_weights_sum();
					if (sum == 0)
						// Avoid UB as sum - 1 has to be >= 0
						continue;
					int rule = uniform_int_distribution<>(0, sum - 1)(s.rng);
					draw_sys(s, r.pick(rule), depth - 1);
				} else if (s.sys.draw(c)) {
					auto p = s.current.pos * s.project;
					s.current.pos += Vector3D::point(1, 0, 0) * s.current.rot;
					auto q = s.current.pos * s.project;
					s.lines.push_back({{p.x, p.y, p.z}, {q.x, q.y, q.z}, s.color});
				}
			}
		}
	}

	static void l_system(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		DrawSystem3D s(lines);
		Cursor3D saved[2048];
		{
			ifstream f(conf["inputfile"].as_string_or_die());
			f >> s.sys;
		}
		s.project = transform_from_conf(conf, mat_project);
		s.color = color_from_conf(conf);
		s.current = { Vector3D::point(0, 0, 0), {} };
		s.drot = Rotation(s.sys.get_angle() / 180 * M_PI);
		s.saved = saved;
		draw_sys(s, s.sys.get_initiator(), s.sys.get_nr_iterations());
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
				line_drawing(fig, mat_eye, lines);
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
			} else if (type == "3DLSystem") {
				l_system(fig, mat_eye, lines);
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
				/*
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
			} else if (type == "3DLSystem") {
				l_system(fig, mat_eye, lines);
				*/
			} else {
				// TODO
				//throw TypeException(type);
			}
		}

		// Draw
		return Triangles3D(triangles).draw(size, bg);
	}
}
