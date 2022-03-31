#include "shapes/buckyball.h"
#include <array>
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/fractal.h"
#include "shapes/icosahedron.h"
#include "math/point3d.h"

using namespace std;

namespace shapes {

	constexpr static array<Point3D, 60> buckyball_points = []() constexpr {
		// Trisect the edges of an icosahedron
		// The two middle points are the points of the buckyball
		Point3D ico_points[30] = {};
		Edge ico_edges[30] = {};
		icosahedron(ico_points);
		icosahedron(ico_edges);
		array<Point3D, 60> points;
		for (int i = 0; i < 30; i++) {
			auto e = ico_edges[i];
			auto a = ico_points[e.a];
			auto b = ico_points[e.b];
			points[i * 2 + 0] = a.interpolate(b, 2.0 / 3);
			points[i * 2 + 1] = a.interpolate(b, 1.0 / 3);
		}
		return points;
	}();

	constexpr static array<Edge, 90> buckyball_edges = []() constexpr {
		array<Edge, 90> edges = {};
		// TODO this is very slow (O(n^2)) but works well enough for now.
		const double limit = 0.3505; // "exact" length is 0.350487
		unsigned int i = 0;
		for (unsigned int u = 0; u < 60; u++) {
			auto p = buckyball_points[u];
			for (unsigned int v = 0; v < u; v++) {
				auto q = buckyball_points[v];
				if ((p - q).length() < limit) {
					assert(i < 90);
					edges[i++] = { u, v };
				}
			}
		}
		return edges;
	}();

	static void buckyball(Face faces[32], const Point3D points[60]) {
	}

	void buckyball(const FigureConfiguration &conf, vector<Line3D> &lines) {
		platonic(conf, lines, buckyball_points, buckyball_edges);
	}

	TriangleFigure buckyball(const FigureConfiguration &conf) {
		vector<Face> faces(32);
		buckyball(faces.data(), buckyball_points.data());
		vector<Point3D> points(buckyball_points.begin(), buckyball_points.end());
		return platonic(conf, points, faces);
	}

	void fractal_buckyball(const FigureConfiguration &conf, vector<Line3D> &lines) {
		vector<Point3D> points(buckyball_points.begin(), buckyball_points.end());
		vector<Edge> edges(buckyball_edges.begin(), buckyball_edges.end());
		fractal(conf, points, edges);
		platonic(conf, lines, points, edges);
	}

	TriangleFigure fractal_buckyball(const FigureConfiguration &conf) {
		vector<Point3D> points(buckyball_points.begin(), buckyball_points.end());
		vector<Face> faces(32);
		buckyball(faces.data(), buckyball_points.data());
		fractal(conf, points, faces);
		return platonic(conf, points, faces);
	}
}
