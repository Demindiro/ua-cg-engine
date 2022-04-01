#pragma once

#include "render/triangle.h"

namespace engine {
namespace render {

struct Frustum {
	double near, far;
	double fov, aspect;

	void clip(TriangleFigure &f) const;
};

}
}
