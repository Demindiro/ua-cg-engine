#pragma once

#include <cmath>
#include "render/triangle.h"

namespace engine {
namespace render {

struct Frustum {
	double near, far;
	double fov, aspect;

	void clip(TriangleFigure &f) const;

	/**
	 * \brief Determine the perspective scaling factor d for this frustum for a given width.
	 */
	double perspective_factor(double width) const {
		return 2 * std::atan(fov / 2) / width;
	}
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
