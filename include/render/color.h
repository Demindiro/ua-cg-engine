#pragma once

#include <algorithm>
#include "easy_image.h"
#include "engine.h"

namespace engine {
namespace render {

struct Color {
	double r, g, b;

	constexpr Color() : r(0), g(0), b(0) {}
	constexpr Color(double r, double g, double b) : r(r), g(g), b(b) {}
	constexpr Color(img::Color c) : r(c.r / 255.0), g(c.g / 255.0), b(c.b / 255.0) {}

	constexpr Color operator +(Color rhs) const {
		return Color { r + rhs.r, g + rhs.g, b + rhs.b };
	}

	constexpr Color operator *(Color rhs) const {
		return Color { r * rhs.r, g * rhs.g, b * rhs.b };
	}

	constexpr Color operator *(double f) const {
		return Color { r * f, g * f, b * f };
	}

	constexpr Color operator /(double f) const {
		return Color { r / f, g / f, b / f };
	}

	constexpr void operator +=(Color rhs) {
		*this = *this + rhs;
	}

	constexpr void operator *=(Color rhs) {
		*this = *this * rhs;
	}

	constexpr Color clamp() const {
		return { std::clamp(r, 0.0, 1.0), std::clamp(g, 0.0, 1.0), std::clamp(b, 0.0, 1.0) };
	}

	inline img::Color to_img_color() const {
		auto c = clamp();
		return img::Color(round_up(c.r * 255), round_up(c.g * 255), round_up(c.b * 255));
	}
};

std::ostream &operator <<(std::ostream &o, const render::Color &c);

}
}
