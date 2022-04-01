#include "shapes/cylinder.h"
#include <vector>
#include "shapes.h"
#include "shapes/circle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

void cylinder(const ini::Section &conf, EdgeShape &shape) {
	auto n = (unsigned int)conf["n"].as_int_or_die();
	auto height = conf["height"].as_double_or_die();

	shape.points.reserve(n * 2);
	circle(shape.points, n, 0);
	circle(shape.points, n, height);

	shape.edges.resize(n * 3);
	for (unsigned int i = 0; i < n; i++) {
		shape.edges[0 * n + i] = { 0 * n + i, 0 * n + (i + 1) % n };
		shape.edges[1 * n + i] = { 1 * n + i, 1 * n + (i + 1) % n };
		shape.edges[2 * n + i] = { 0 * n + i, 1 * n + i };
	}
}

void cylinder_sides(unsigned int n, double height, EdgeShape &shape) {
	shape.points.reserve(n * 2);
	circle(shape.points, n, 0);
	circle(shape.points, n, height);

	shape.edges.resize(n);
	for (unsigned int i = 0; i < n; i++) {
		shape.edges[i] = { i, n + i };
	}
}

void cylinder(const ini::Section &conf, FaceShape &f) {
	auto n = (unsigned int)conf["n"].as_int_or_die();
	auto height = conf["height"].as_double_or_die();

	f.points.reserve(n * 2);
	f.faces.reserve(n * 4);
	circle(f.points, n, 0);
	circle(f.points, n, height);
	circle(f.faces, n, 0);
	circle_reversed(f.faces, n, n);

	for (unsigned int i = 0; i < n; i++) {
		unsigned int j = (i + 1) % n;
		f.faces.push_back({ j, i, n + i });
		f.faces.push_back({ j, n + i, n + j });
	}
}

void cylinder_sides(unsigned int n, double height, FaceShape &f) {
	f.points.reserve(n * 2);
	circle(f.points, n, 0);
	circle(f.points, n, height);

	f.faces.reserve(n * 2);
	for (unsigned int i = 0; i < n; i++) {
		unsigned int j = (i + 1) % n;
		f.faces.push_back({ j, i, n + i });
		f.faces.push_back({ j, n + i, n + j });
	}
}

}
}
