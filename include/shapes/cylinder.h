#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void cylinder(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure cylinder(const FigureConfiguration &conf);
}
