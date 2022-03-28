#include "shapes/cylinder.h"
#include <vector>
#include "shapes.h"
#include "shapes/circle.h"

using namespace std;

namespace shapes {
	void cylinder(const FigureConfiguration &conf, vector<Line3D> &lines) {
		auto n = (unsigned int)conf.section["n"].as_int_or_die();
		auto height = conf.section["height"].as_double_or_die();

		vector<Point3D> points;
		points.reserve(n * 2);
		circle(points, n, 0);
		circle(points, n, height);

		vector<Edge> edges(n * 3);
		for (unsigned int i = 0; i < n; i++) {
			edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
			edges[1 * n + i] = { 1 * n + i, 1 * n + (i + 1) % n };
			edges[2 * n + i] = { 0 * n + i, 1 * n + i };
		}

		platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	TriangleFigure cylinder(const FigureConfiguration &conf) {
		auto n = (unsigned int)conf.section["n"].as_int_or_die();
		auto height = conf.section["height"].as_double_or_die();

		vector<Point3D> points;
		vector<Face> faces;
		points.reserve(n * 2);
		faces.reserve(n * 2);
		circle(points, n, 0);
		circle(points, n, height);
		circle(faces, n, 0);
		circle_reversed(faces, n, n);

		for (unsigned int i = 0; i < n; i++) {
			unsigned int j = (i + 1) % n;
			faces.push_back({ j, i, n + i });
			faces.push_back({ j, n + i, n + j });
		}

		return platonic(conf, points, faces);
	}
}
