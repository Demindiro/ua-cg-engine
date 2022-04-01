#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void cube(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure cube(const FigureConfiguration &conf);

void fractal_cube(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure fractal_cube(const FigureConfiguration &conf);

}
}
