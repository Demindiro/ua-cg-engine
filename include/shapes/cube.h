#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

const ShapeTemplate<8, 12, 12> cube {
	{{
		{  1,  1,  1 },
		{  1,  1, -1 },
		{  1, -1,  1 },
		{  1, -1, -1 },
		{ -1,  1,  1 },
		{ -1,  1, -1 },
		{ -1, -1,  1 },
		{ -1, -1, -1 },
	}},
	{{
		// X
		{ 0, 4 },
		{ 1, 5 },
		{ 2, 6 },
		{ 3, 7 },
		// Y
		{ 0, 2 },
		{ 1, 3 },
		{ 4, 6 },
		{ 5, 7 },
		// Z
		{ 0, 1 },
		{ 2, 3 },
		{ 4, 5 },
		{ 6, 7 },
	}},
	{{
		// X
		{ 1, 0, 2 },
		{ 3, 1, 2 },
		{ 4, 5, 6 },
		{ 7, 6, 5 },
		// Y
		{ 0, 1, 4 },
		{ 1, 5, 4 },
		{ 3, 2, 6 },
		{ 3, 6, 7 },
		// Z
		{ 2, 0, 4 },
		{ 6, 2, 4 },
		{ 1, 3, 5 },
		{ 7, 5, 3 },
	}},
};

}
}
