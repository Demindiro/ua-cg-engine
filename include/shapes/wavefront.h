#pragma once

#include "ini_configuration.h"
#include "shapes.h"

namespace engine {
namespace shapes {

void wavefront(const ini::Section &conf, FaceShape &shape);

}
}
