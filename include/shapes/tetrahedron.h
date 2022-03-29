#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "vector3d.h"

namespace shapes {
	void tetrahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure tetrahedron(const FigureConfiguration &conf);

	void fractal_tetrahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure fractal_tetrahedron(const FigureConfiguration &conf);
}
