#include "shapes/buckyball.h"
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/fractal.h"
#include "shapes/icosahedron.h"
#include "math/point3d.h"

using namespace std;

namespace shapes {
	static void buckyball(Point3D points[60]) {
		// Trisect the edges of an icosahedron
		// The two middle points are the points of the buckyball
		Point3D ico_points[30];
		Edge ico_edges[30];
		icosahedron(ico_points);
		icosahedron(ico_edges);
		for (int i = 0; i < 30; i++) {
			auto e = ico_edges[i];
			auto a = ico_points[e.a];
			auto b = ico_points[e.b];
			points[i * 2 + 0] = a.interpolate(b, 2.0 / 3);
			points[i * 2 + 1] = a.interpolate(b, 1.0 / 3);
		}
	}

	static void buckyball(Edge edges[90], Point3D points[60]) {
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

	static void buckyball(Face faces[32], Point3D points[60]) {
	}

	void buckyball(const FigureConfiguration &conf, vector<Line3D> &lines) {
		Point3D points[60];
		Edge edges[90];
		buckyball(points);
		buckyball(edges, points);
		platonic(conf, lines, points, 60, edges, 90);
	}

	TriangleFigure buckyball(const FigureConfiguration &conf) {
		vector<Point3D> points(60);
		vector<Face> faces(32);
		buckyball(points.data());
		buckyball(faces.data(), points.data());
		return platonic(conf, points, faces);
	}

	void fractal_buckyball(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points(60);
		vector<Edge> edges(90);
		buckyball(points.data());
		buckyball(edges.data(), points.data());
		fractal(conf, points, edges);
		platonic(conf, lines, points, edges);
	}

	TriangleFigure fractal_buckyball(const FigureConfiguration &conf) {
		vector<Point3D> points(60);
		vector<Face> faces(32);
		buckyball(points.data());
		buckyball(faces.data(), points.data());
		fractal(conf, points, faces);
		return platonic(conf, points, faces);
	}
}
