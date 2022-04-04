#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "render/triangle.h"
#include "shapes.h"

namespace engine {
namespace shapes {

void cylinder_sides(unsigned int n, double height, EdgeShape &);

void cylinder_sides(unsigned int n, double height, FaceShape &, bool point_normals);

void cylinder(const Configuration &, EdgeShape &);

void cylinder(const Configuration &, FaceShape &);

}
}
