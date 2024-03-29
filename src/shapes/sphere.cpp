#include "shapes/sphere.h"
#include <vector>
#include "shapes.h"
#include "shapes/icosahedron.h"
#include "math/vector3d.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

static void bisect(vector<Point3D> &points, vector<Edge> &edges, vector<Face> &faces) {
	vector<Edge> new_edges;
	vector<Face> new_faces;
	new_edges.reserve(edges.size() * 3);
	new_faces.reserve(faces.size() * 4);

	map<Edge, unsigned int> edge_to_new_point;

	// Use edges to add points & determine new edges
	for (auto e : edges) {
		unsigned int i = points.size();
		points.push_back(Point3D::center({ points[e.a], points[e.b] }));
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
		new_faces.push_back({  d,   e,   f});
		new_faces.push_back({g.a,   d,   f});
		new_faces.push_back({  d, g.b,   e});
		new_faces.push_back({  f,   e, g.c});
		new_edges.push_back({ d, e });
		new_edges.push_back({ e, f });
		new_edges.push_back({ f, d });
	}

	edges = new_edges;
	faces = new_faces;
}

static void sphere(unsigned int n, vector<Point3D> &points, vector<Edge> &edges, vector<Face> &faces) {
	points = { icosahedron.points.begin(), icosahedron.points.end() };
	edges = { icosahedron.edges.begin(), icosahedron.edges.end() };
	faces = { icosahedron.faces.begin(), icosahedron.faces.end() };

	for (unsigned int i = 0; i < n; i++) {
		bisect(points, edges, faces);
	}

	for (auto &p : points) {
		p = Point3D(Vector3D(p.x, p.y, p.z).normalize());
	}
}

void sphere(unsigned int n, EdgeShape &f) {
	vector<Face> faces;
	sphere(n, f.points, f.edges, faces);
}

void sphere(unsigned int n, FaceShape &f, bool point_normals) {
	vector<Edge> edges;
	sphere(n, f.points, edges, f.faces);
	if (point_normals) {
		f.normals.reserve(f.points.size());
		for (auto p : f.points) {
			f.normals.push_back(p.to_vector());
		}
	}
}

void sphere(const Configuration &conf, EdgeShape &f) {
	sphere((unsigned int)conf.section["n"].as_int_or_die(), f);
}

void sphere(const Configuration &conf, FaceShape &f) {
	sphere((unsigned int)conf.section["n"].as_int_or_die(), f, conf.point_normals);
}

}
}
