#include "shapes/mengersponge.h"
#include "math/point3d.h"
#include "shapes.h"
#include "shapes/cube.h"

#include <iostream>
using namespace std;

namespace engine {
namespace shapes {

using namespace render;

/**
 * \brief Determine the positions of the cubes in the mengersponge.
 *
 * This accepts a templated function to avoid any redundant allocations.
 */
template<typename F>
static void gen_points(int n, F f, Point3D base = {}, double size = 1.0) {
	if (n == 0) {
		f(base, size);
		return;
	}
	// |-----x-----|
	// |-x-|-x-|-x-|
	//  ... ... ...
	size /= 3;
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			for (int z = -1; z <= 1; z++) {
				if ((x == 0 && y == 0) || (x == 0 && z == 0) || (y == 0 && z == 0)) {
					continue;
				}
				gen_points(n - 1, f, base + Vector3D(x, y, z) * 2 * size, size);
			}
		}
	}
}

void mengersponge(const Configuration &conf, EdgeShape &shape) {
	gen_points(conf.section["nrIterations"].as_int_or_die(), [&](auto orig, auto size) {
		auto o = (unsigned int)shape.points.size();
		for (auto e : cube.edges) { 
			shape.edges.push_back({ o + e.a, o + e.b });
		}
		for (auto p : cube.points) { 
			shape.points.push_back(orig + p.to_vector() * size);
		}
	});
}

void mengersponge(const Configuration &conf, FaceShape &shape) {
	gen_points(conf.section["nrIterations"].as_int_or_die(), [&](auto orig, auto size) {
		auto o = (unsigned int)shape.points.size();
		for (auto f : cube.faces) { 
			shape.faces.push_back({ o + f.a, o + f.b, o + f.c });
		}
		for (auto p : cube.points) { 
			shape.points.push_back(orig + p.to_vector() * size);
		}
	});
}

}
}
