#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

constexpr ShapeTemplate<4, 6, 4> tetrahedron {
	{{
		{  1, -1, -1 },
		{ -1,  1, -1 },
		{  1,  1,  1 },
		{ -1, -1,  1 },
	}},
	{{
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 3 },
		{ 1, 2 },
		{ 1, 3 },
		{ 2, 3 },
	}},
	{{
		{ 0, 1, 2 },
		{ 0, 3, 1 },
		{ 0, 2, 3 },
		{ 2, 1, 3 },
	}},
};

}
}
