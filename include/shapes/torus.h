#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void torus(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure torus(const FigureConfiguration &conf);

}
}
