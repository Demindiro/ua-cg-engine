#pragma once

#include <vector>
#include "ini_configuration.h"
#include "math/matrix4d.h"
#include "lines.h"
#include "shapes.h"

namespace engine {
namespace wireframe {

void l_system(const ini::Section &, shapes::EdgeShape &);

void line_drawing(const ini::Section &, shapes::EdgeShape &);

}
}
