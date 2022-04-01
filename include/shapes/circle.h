#pragma once

#include <vector>
#include "math/point3d.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {
	void circle(std::vector<Point3D> &points, unsigned int n, double z);

	void circle(std::vector<render::Face> &faces, unsigned int n, unsigned int offt);

	void circle_reversed(std::vector<render::Face> &faces, unsigned int n, unsigned int offt);
}
}
