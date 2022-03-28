#pragma once

#include <vector>
#include "vector3d.h"
#include "shapes.h"

namespace shapes {
	void circle(std::vector<Point3D> &points, unsigned int n, double z);

	void circle(std::vector<Face> &faces, unsigned int n, unsigned int offt);
}
