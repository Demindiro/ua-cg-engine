#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "render/triangle.h"
#include "shapes.h"

namespace engine {
namespace shapes {
	void cone(const ini::Section &, EdgeShape &);

	void cone(const ini::Section &, FaceShape &);
}
}
