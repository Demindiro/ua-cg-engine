#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "math/point3d.h"
#include "math/vector3d.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

void fractal(const Configuration &, const ShapeTemplateAny &, EdgeShape &);

void fractal(const Configuration &, const ShapeTemplateAny &, FaceShape &);

}
}
