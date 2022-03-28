#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void sphere(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure sphere(ini::Section &conf, Matrix &mat_project);
}
