#pragma once

#include <vector>
#include "vector3d.h"
#include "shapes.h"

namespace shapes {
	void circle(std::vector<Vector3D> &points, int n, double z);

	void circle(std::vector<Face> &faces, int n, int offt);
}
