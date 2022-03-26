#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace wireframe {
	void l_system(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	void line_drawing(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);
}
