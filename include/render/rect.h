#pragma once

#include <cmath>
#include <ostream>
#include "math/point2d.h"
#include "math/vector2d.h"

namespace engine {
namespace render {

struct Rect {
	Point2D min, max;

	constexpr Rect operator |(const Rect &with) const {
		return {
			{ std::min(min.x, with.min.x), std::min(min.y, with.min.y) },
			{ std::max(max.x, with.max.x), std::max(max.y, with.max.y) },
		};
	}

	constexpr Rect operator |(const Point2D &with) const {
		return *this | Rect { with, with };
	}

	constexpr Rect operator |=(const Rect &with) {
		return *this = *this | with;
	}

	constexpr Rect operator |=(const Point2D &with) {
		return *this = *this | with;
	}

	constexpr Vector2D size() const {
		return max.to_vector() - min.to_vector();
	}
};

std::ostream &operator <<(std::ostream &out, const Rect &rect);

}
}
