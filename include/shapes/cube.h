#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void cube(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure cube(ini::Section &conf, Matrix &mat_project);

	void fractal_cube(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure  fractal_cube(ini::Section &conf, Matrix &mat_project);
}
