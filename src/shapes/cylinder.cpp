#include "shapes/cylinder.h"
#include <vector>
#include "shapes.h"
#include "shapes/circle.h"

using namespace std;

namespace shapes {
	void cylinder(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
		auto n = (unsigned int)conf["n"].as_int_or_die();
		auto height = conf["height"].as_double_or_die();

		vector<Vector3D> points;
		points.reserve(n * 2);
		circle(points, n, 0);
		circle(points, n, height);

		vector<Edge> edges(n * 3);
		for (unsigned int i = 0; i < n; i++) {
			edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
			edges[1 * n + i] = { 1 * n + i, 1 * n + (i + 1) % n };
			edges[2 * n + i] = { 0 * n + i, 1 * n + i };
		}

		platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	void cylinder(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
		auto n = (unsigned int)conf["n"].as_int_or_die();
		auto height = conf["height"].as_double_or_die();

		vector<Vector3D> points;
		vector<Face> faces;
		points.reserve(n * 2);
		faces.reserve(n * 2);
		circle(points, n, 0);
		circle(points, n, height);
		circle(faces, n, 0);
		circle(faces, n, n);

		for (unsigned int i = 0; i < n; i++) {
			unsigned int j = (i + 1) % n;
			faces.push_back({ i, j, n + i });
			faces.push_back({ j, n + i, n + j });
		}

		platonic(conf, mat_project, triangles, points.data(), points.size(), faces.data(), faces.size());
	}
}
