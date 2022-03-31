#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"

namespace shapes {
	void buckyball(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure buckyball(const FigureConfiguration &conf);

	void fractal_buckyball(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	TriangleFigure fractal_buckyball(const FigureConfiguration &conf);
}
