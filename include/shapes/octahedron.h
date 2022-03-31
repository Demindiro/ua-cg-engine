#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"

namespace shapes {
	void octahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure octahedron(const FigureConfiguration &conf);

	void fractal_octahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure fractal_octahedron(const FigureConfiguration &conf);
}
