#pragma once

#include <initializer_list>
#include <ostream>
#include "math/vector2d.h"

struct Point2D {
	double x, y;

	constexpr inline Point2D() : x(0), y(0) {}

	constexpr inline Point2D(double x, double y) : x(x), y(y) {}

	constexpr inline Point2D(Vector2D v) : x(v.x), y(v.y) {}

	constexpr inline Point2D operator +(Vector2D v) const {
		return { x + v.x, y + v.y };
	}

	constexpr inline Point2D &operator +=(Vector2D v) {
		return *this = *this + v;
	}

	constexpr inline Point2D operator -(Vector2D v) const {
		return { x - v.x, y - v.y };
	}

	constexpr inline Point2D &operator -=(Vector2D v) {
		return *this = *this - v;
	}

	constexpr inline Vector2D operator -(Point2D rhs) const {
		return { x - rhs.x, y - rhs.y };
	}

	constexpr inline Point2D interpolate(Point2D to, double f) const {
		return {
			x * (1 - f) + to.x * f,
			y * (1 - f) + to.y * f,
		};
	}

	constexpr inline static Point2D center(std::initializer_list<Point2D> points) {
		Vector2D s;
		for (auto p : points) {
			s += p.to_vector();
		}
		return Point2D(s / points.size());
	}

	constexpr inline Vector2D to_vector() const {
		return Vector2D(x, y);
	}
};

std::ostream &operator <<(std::ostream &o, const Point2D &m);
