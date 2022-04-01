#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void octahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure octahedron(const FigureConfiguration &conf);

void fractal_octahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure fractal_octahedron(const FigureConfiguration &conf);

}
}
