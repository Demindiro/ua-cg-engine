#pragma once

#include "render/triangle.h"

namespace engine {
namespace render {

struct Frustum {
	double near, far;
	double fov, aspect;

	void clip(TriangleFigure &f) const;
};

constexpr Point2D project(Point3D p) {
	assert(p.z != 0 && "division by 0");
	return { p.x / -p.z, p.y / -p.z };
}

constexpr Point2D project(Point3D p, double d, Vector2D offset) {
	return Point2D(project(p).to_vector() * d + offset);
}

}
}
