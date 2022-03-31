#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"

namespace shapes {
	void cone(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure cone(const FigureConfiguration &conf);
}
