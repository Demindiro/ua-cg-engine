#pragma once

#include "math/point3d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "render/color.h"
#include "render/light.h"
#include "render/triangle.h"
#include "render/lines.h"

namespace engine {
namespace render {

Matrix4D look_direction(Point3D pos, Vector3D dir, Matrix4D &inv);

Matrix4D look_direction(Point3D pos, Vector3D dir);

img::EasyImage draw(std::vector<TriangleFigure> figures, Lights lights, unsigned int size, Color background);

img::EasyImage draw(std::vector<LineFigure> figures, unsigned int size, Color background, bool with_z);

}
}
