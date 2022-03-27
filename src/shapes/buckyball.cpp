#include "shapes/buckyball.h"
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/fractal.h"
#include "shapes/icosahedron.h"
#include "vector3d.h"

using namespace std;

namespace shapes {
	static void buckyball(Vector3D points[60]) {
		// Trisect the edges of an icosahedron
		// The two middle points are the points of the buckyball
		Vector3D ico_points[30];
		Edge ico_edges[30];
		icosahedron(ico_points);
		icosahedron(ico_edges);
		for (int i = 0; i < 30; i++) {
			auto e = ico_edges[i];
			auto a = ico_points[e.a];
			auto b = ico_points[e.b];
			points[i * 2 + 0] = (a + b * 2) / 3;
			points[i * 2 + 1] = (a * 2 + b) / 3;
		}
	}

	static void buckyball(Edge edges[90], Vector3D points[60]) {
		// TODO this is very slow (O(n^2)) but works well enough for now.
		const double limit = 0.3505; // "exact" length is 0.350487
		unsigned int i = 0;
		for (unsigned int u = 0; u < 60; u++) {
			auto p = points[u];
			for (unsigned int v = 0; v < u; v++) {
				auto q = points[v];
				if ((p - q).length() < limit) {
					assert(i < 90);
					edges[i++] = { u, v };
				}
			}
		}
	}

	static void buckyball(Face faces[32], Vector3D points[60]) {
	}

	void buckyball(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		Vector3D points[60];
		Edge edges[90];
		buckyball(points);
		buckyball(edges, points);
		platonic(conf, mat_project, lines, points, 60, edges, 90);
	}

	void buckyball(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		Vector3D points[60];
		Face faces[32];
		buckyball(points);
		buckyball(faces, points);
		platonic(conf, mat_project, triangles, points, 60, faces, 32);
	}

	void fractal_buckyball(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		vector<Vector3D> points(60);
		vector<Edge> edges(90);
		buckyball(points.data());
		buckyball(edges.data(), points.data());
		fractal(conf, points, edges);
		platonic(conf, mat_project, lines, points, edges);
	}

	void fractal_buckyball(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		vector<Vector3D> points(60);
		vector<Face> faces(32);
		buckyball(points.data());
		buckyball(faces.data(), points.data());
		fractal(conf, points, faces);
		platonic(conf, mat_project, triangles, points, faces);
	}
}
