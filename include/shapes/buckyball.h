#pragma once

#include <array>
#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "shapes/icosahedron.h"

namespace engine {
namespace shapes {

const ShapeTemplate<60, 90, 32> buckyball(
	[]() constexpr {
		// Trisect the edges of an icosahedron
		// The two middle points are the points of the buckyball
		std::array<Point3D, 60> points;
		for (int i = 0; i < 30; i++) {
			auto e = icosahedron.edges[i];
			auto a = icosahedron.points[e.a];
			auto b = icosahedron.points[e.b];
			points[i * 2 + 0] = a.interpolate(b, 2.0 / 3);
			points[i * 2 + 1] = a.interpolate(b, 1.0 / 3);
		}
		return points;
	},
	[](auto points) constexpr {
		std::array<render::Edge, 90> edges;
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
		return edges;
	},
	[](auto, auto) constexpr {
		std::array<render::Face, 32> faces;
		return faces;
	}
);

}
}
