#pragma once

#include "ini_configuration.h"
#include "shapes.h"

namespace engine {
namespace shapes {

void sphere(unsigned int n, EdgeShape &);

void sphere(unsigned int n, FaceShape &);

void sphere(const ini::Section &, EdgeShape &);

void sphere(const ini::Section &, FaceShape &);

}
}
