#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace shapes {
	void cylinder(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	void cylinder(ini::Section &conf, Matrix &mat_project, std::vector<Triangle3D> &triangles);
}
