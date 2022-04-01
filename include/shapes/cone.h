#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "render/triangle.h"
#include "shapes.h"

namespace engine {
namespace shapes {
	void cone(const FigureConfiguration &conf, std::vector<Line3D> &lines);

	render::TriangleFigure cone(const FigureConfiguration &conf);
}
}
