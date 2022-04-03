#pragma once

#include <vector>
#include "ini_configuration.h"
#include "lines.h"
#include "shapes.h"
#include "render/triangle.h"

namespace engine {
namespace shapes {

constexpr ShapeTemplate<6, 12, 8> octahedron {
	{{
		{  1,  0,  0 },
		{ -1,  0,  0 },
		{  0,  1,  0 },
		{  0,  0,  1 },
		{  0, -1,  0 },
		{  0,  0, -1 },
	}},
	{{
		// Top
		{ 0, 2 },
		{ 0, 3 },
		{ 0, 4 },
		{ 0, 5 },
		// Bottom
		{ 1, 2 },
		{ 1, 3 },
		{ 1, 4 },
		{ 1, 5 },
		// Ring
		{ 2, 3 },
		{ 3, 4 },
		{ 4, 5 },
		{ 5, 2 },
	}},
	{{
		// Top
		{ 0, 2, 3 },
		{ 0, 3, 4 },
		{ 0, 4, 5 },
		{ 0, 5, 2 },
		// Bottom
		{ 2, 1, 3 },
		{ 3, 1, 4 },
		{ 4, 1, 5 },
		{ 5, 1, 2 },
	}}
};

}
}
