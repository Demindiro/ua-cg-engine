#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "render/lines.h"
#include "render/triangle.h"
#include "shapes.h"

namespace engine {
namespace shapes {

void thicken(const Configuration &, const ShapeTemplateAny &, EdgeShape &);

void thicken(const Configuration &, const ShapeTemplateAny &, FaceShape &);

}
}
