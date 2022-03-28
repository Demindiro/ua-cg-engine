#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void buckyball(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure buckyball(ini::Section &conf, Matrix &mat_project);

	void fractal_buckyball(ini::Section &conf, Matrix &mat_project, std::vector<Line3D> &lines);

	TriangleFigure fractal_buckyball(ini::Section &conf, Matrix &mat_project);
}
