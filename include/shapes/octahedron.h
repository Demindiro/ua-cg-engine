#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void octahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure octahedron(ini::Section &conf, Matrix &mat_project);

	void fractal_octahedron(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure fractal_octahedron(ini::Section &conf, Matrix &mat_project);
}
