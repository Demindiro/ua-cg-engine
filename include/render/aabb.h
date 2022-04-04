#pragma once

#include <cmath>
#include "math/point3d.h"
#include "math/vector3d.h"

namespace engine {
namespace render {

struct Aabb {
	Point3D min, max;

	constexpr Aabb operator |(const Aabb &with) const {
		return {
			{
				std::min(min.x, with.min.x),
				std::min(min.y, with.min.y),
				std::min(min.z, with.min.z),
			},
			{
				std::max(max.x, with.max.x),
				std::max(max.y, with.max.y),
				std::max(max.z, with.max.z),
			},
		};
	}

	constexpr Aabb operator |(const Point3D &with) const {
		return *this | Aabb { with, with };
	}

	constexpr Aabb operator |=(const Aabb &with) {
		return *this = *this | with;
	}

	constexpr Aabb operator |=(const Point3D &with) {
		return *this = *this | with;
	}

	constexpr Vector3D size() const {
		return max.to_vector() - min.to_vector();
	}
};

}
}
