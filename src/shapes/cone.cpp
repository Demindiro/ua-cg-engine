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

void cone(const ini::Section &conf, EdgeShape &shape) {
	auto n = (unsigned int)conf["n"].as_int_or_die();
	auto height = conf["height"].as_double_or_die();

	shape.points.reserve(n + 1);
	circle(shape.points, n, 0);
	shape.points.push_back({ 0, 0, height });

	shape.edges.resize(n * 2);
	for (unsigned int i = 0; i < n; i++) {
		shape.edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
		shape.edges[1 * n + i] = { 0 * n + i, n };
	}
}

void cone(const ini::Section &conf, FaceShape &shape) {
	auto n = (unsigned int)conf["n"].as_int_or_die();
	auto height = conf["height"].as_double_or_die();

	auto &shape_faces = *shape.faces;

	shape.points.reserve(n + 1);
	shape_faces.reserve(n * 2);
	circle(shape.points, n, 0);
	circle(shape_faces, n, 0);
	shape.points.push_back({ 0, 0, height });

	for (unsigned int i = 0; i < n; i++) {
		shape_faces.push_back({ i, n, (i + 1) % n });
	}
}

}
}
