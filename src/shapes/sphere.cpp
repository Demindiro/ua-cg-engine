#include "shapes/sphere.h"
#include <vector>
#include "shapes.h"
#include "shapes/icosahedron.h"
#include "vector3d.h"

using namespace std;

namespace shapes {
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

	static void sphere(ini::Section &conf, vector<Vector3D> &points, vector<Edge> &edges, vector<Face> &faces) {
		auto n = conf["n"].as_int_or_die();
		points = vector<Vector3D>(12);
		edges = vector<Edge>(30);
		faces = vector<Face>(20);

		icosahedron(points.data());
		icosahedron(edges.data());
		icosahedron(faces.data());

		for (int i = 0; i < n; i++) {
			bisect(points, edges, faces);
		}

		for (auto &p : points) {
			p.normalise();
		}
	}

	void sphere(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		vector<Vector3D> points;
		vector<Edge> edges;
		vector<Face> faces;
		sphere(conf, points, edges, faces);
		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	void sphere(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		vector<Vector3D> points;
		vector<Edge> edges;
		vector<Face> faces;
		sphere(conf, points, edges, faces);
		platonic(conf, mat_project, triangles, points.data(), points.size(), faces.data(), faces.size());
	}
}
