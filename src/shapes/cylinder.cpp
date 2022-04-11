#include "shapes/cylinder.h"
#include <vector>
#include "shapes.h"
#include "shapes/circle.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

void cylinder(const Configuration &conf, EdgeShape &shape) {
	auto n = (unsigned int)conf.section["n"].as_int_or_die();
	auto height = conf.section["height"].as_double_or_die();

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

void cylinder(unsigned int n, double height, FaceShape &f, bool point_normals) {
	f.points.reserve(point_normals ? n * 4 : n * 2);
	f.faces.reserve(n * 4);
	circle(f.points, n, 0);
	circle(f.points, n, height);
	if (point_normals) {
		circle(f.points, n, 0);
		circle(f.points, n, height);
	}

	for (unsigned int i = 0; i < n; i++) {
		unsigned int j = (i + 1) % n;
		f.faces.push_back({ j, i, n + i });
		f.faces.push_back({ j, n + i, n + j });
	}
	unsigned int offset = point_normals ? n * 2 : 0;
	circle(f.faces, n, 0 + offset);
	circle_reversed(f.faces, n, n + offset);
	
	if (point_normals) {
		f.normals.resize(n * 4);
		Rotation d(-2 * M_PI / n), r;
		for (unsigned int i = 0; i < n; i++) {
			f.normals[i + 0 * n] = { r.u, r.v, 0 };
			f.normals[i + 1 * n] = { r.u, r.v, 0 };
			r *= d;
		}
		for (unsigned int i = 0; i < n; i++) {
			f.normals[i + 2 * n] = { 0, 0, -1 };
		}
		for (unsigned int i = 0; i < n; i++) {
			f.normals[i + 3 * n] = { 0, 0, 1 };
		}
	}
}
void cylinder(const Configuration &conf, FaceShape &f) {
	cylinder((unsigned int)conf.section["n"].as_int_or_die(), conf.section["height"].as_double_or_die(), f, conf.point_normals);
}


void cylinder_sides(unsigned int n, double height, FaceShape &f, bool point_normals) {
	f.points.reserve(n * 2);
	circle(f.points, n, 0);
	circle(f.points, n, height);

	f.faces.reserve(n * 2);
	for (unsigned int i = 0; i < n; i++) {
		unsigned int j = (i + 1) % n;
		f.faces.push_back({ j, i, n + i });
		f.faces.push_back({ j, n + i, n + j });
	}
	
	if (point_normals) {
		f.normals.resize(n * 2);
		Rotation d(-2 * M_PI / n), r;
		for (unsigned int i = 0; i < n; i++) {
			f.normals[i + 0 * n] = { r.u, r.v, 0 };
			f.normals[i + 1 * n] = { r.u, r.v, 0 };
			r *= d;
		}
	}
}

}
}
