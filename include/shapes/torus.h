#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void torus(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure torus(const FigureConfiguration &conf);
}
