#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void sphere(const FigureConfiguration &conf, std::vector<Line3D> &lines);

render::TriangleFigure sphere(const FigureConfiguration &conf);

}
}
