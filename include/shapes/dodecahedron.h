#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"

namespace shapes {
	void dodecahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure dodecahedron(const FigureConfiguration &conf);

	void fractal_dodecahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure fractal_dodecahedron(const FigureConfiguration &conf);
}
