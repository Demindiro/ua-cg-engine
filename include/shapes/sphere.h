#pragma once

#include "ini_configuration.h"
#include "shapes.h"

namespace engine {
namespace shapes {

void sphere(unsigned int n, EdgeShape &);

void sphere(unsigned int n, FaceShape &, bool point_normals);

void sphere(const Configuration &, EdgeShape &);

void sphere(const Configuration &, FaceShape &);

}
}
