#include "shapes/cone.h"
#include <vector>
#include "shapes.h"
#include "shapes/circle.h"
#include "math/point3d.h"
#include "math/vector3d.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

void cone(const FigureConfiguration &conf, vector<Line3D> &lines) {
	auto n = (unsigned int)conf.section["n"].as_int_or_die();
	auto height = conf.section["height"].as_double_or_die();

	vector<Point3D> points;
	points.reserve(n + 1);
	circle(points, n, 0);
	points.push_back({ 0, 0, height });

	vector<Edge> edges(n * 2);
	for (unsigned int i = 0; i < n; i++) {
		edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
		edges[1 * n + i] = { 0 * n + i, n };
	}

	platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
}

TriangleFigure cone(const FigureConfiguration &conf) {
	auto n = (unsigned int)conf.section["n"].as_int_or_die();
	auto height = conf.section["height"].as_double_or_die();

	vector<Point3D> points;
	vector<Face> faces;
	points.reserve(n + 1);
	faces.reserve(n * 2);
	circle(points, n, 0);
	circle(faces, n, 0);
	points.push_back({ 0, 0, height });

	for (unsigned int i = 0; i < n; i++) {
		faces.push_back({ i, n, (i + 1) % n });
	}

	return platonic(conf, points, faces);
}

}
}
