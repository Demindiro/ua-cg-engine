#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void dodecahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure dodecahedron(const FigureConfiguration &conf);

void fractal_dodecahedron(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure fractal_dodecahedron(const FigureConfiguration &conf);

}
}
