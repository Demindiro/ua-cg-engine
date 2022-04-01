#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void tetrahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure tetrahedron(const FigureConfiguration &conf);

void fractal_tetrahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure fractal_tetrahedron(const FigureConfiguration &conf);

}
}
