#pragma once

#include <vector>

namespace engine {
namespace render {

struct Edge {
	unsigned int a, b;

	// TODO this is only necessary for bisect, but it certainly can be done without bisect.
	bool operator<(const Edge &rhs) const {
		auto ll = std::min(a, b), lh = std::max(a, b);
		auto rl = std::min(rhs.a, rhs.b), rh = std::max(rhs.a, rhs.b);
		return ll == rl ? lh < rh : ll < rl;
	}
};

struct LineFigure {
	std::vector<Point3D> points;
	std::vector<Edge> edges;
	Color color;
};

}
}
