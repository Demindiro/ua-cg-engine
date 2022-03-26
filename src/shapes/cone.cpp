#include "shapes/cone.h"
#include <vector>
#include "shapes.h"
#include "shapes/circle.h"
#include "vector3d.h"

using namespace std;

void shapes::cone(ini::Section &conf, Matrix &mat_project, vector<Line3D> &lines) {
	auto n = (unsigned int)conf["n"].as_int_or_die();
	auto height = conf["height"].as_double_or_die();

	vector<Vector3D> points;
	points.reserve(n + 1);
	circle(points, n, 0);
	points.push_back(Vector3D::point(0, 0, height));

	vector<Edge> edges(n * 2);
	for (unsigned int i = 0; i < n; i++) {
		edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
		edges[1 * n + i] = { 0 * n + i, n };
	}

	platonic(conf, mat_project, lines, points.data(), points.size(), edges.data(), edges.size());
}

void shapes::cone(ini::Section &conf, Matrix &mat_project, vector<Triangle3D> &triangles) {
	auto n = (unsigned int)conf["n"].as_int_or_die();
	auto height = conf["height"].as_double_or_die();

	vector<Vector3D> points;
	vector<Face> faces;
	points.reserve(n + 1);
	faces.reserve(n * 2);
	circle(points, n, 0);
	circle(faces, n, 0);
	points.push_back(Vector3D::point(0, 0, height));

	for (unsigned int i = 0; i < n; i++) {
		faces.push_back({ n, i, (i + 1) % n });
	}

	platonic(conf, mat_project, triangles, points.data(), points.size(), faces.data(), faces.size());
}

