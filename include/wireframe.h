#pragma once

#include <vector>
#include "ini_configuration.h"
#include "math/matrix4d.h"
#include "lines.h"

namespace wireframe {
	void l_system(ini::Section &conf, Matrix4D &mat_project, std::vector<Line3D> &lines);

	void line_drawing(ini::Section &conf, Matrix4D &mat_project, std::vector<Line3D> &lines);
}
