#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void cube(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure cube(const FigureConfiguration &conf);

	void fractal_cube(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure  fractal_cube(const FigureConfiguration &conf);
}
