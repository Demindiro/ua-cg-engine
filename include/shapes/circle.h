#pragma once

#include <vector>
#include "math/point3d.h"

namespace shapes {
	void circle(std::vector<Point3D> &points, unsigned int n, double z);

	void circle(std::vector<Face> &faces, unsigned int n, unsigned int offt);

	void circle_reversed(std::vector<Face> &faces, unsigned int n, unsigned int offt);
}
