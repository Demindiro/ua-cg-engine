#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace shapes {
	void cone(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);
	void cone(ini::Section &conf, Matrix &mat_project, std::vector<Triangle3D> &triangles);
}
