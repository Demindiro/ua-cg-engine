#pragma once

#include <cassert>
#include <cmath>

struct Vector2D {

	double x, y;

	constexpr Vector2D() : x(0), y(0) {}

	constexpr Vector2D(double x, double y) : x(x), y(y) {}

	constexpr double operator [](unsigned int row) const {
		assert(row < 2);
		double e[2] = { x, y };
		return e[row];
	}

	constexpr double &operator [](unsigned int row) {
		assert(row < 2);
		double *e[2] = { &x, &y };
		return *e[row];
	}

	constexpr Vector2D operator +() const {
		return *this;
	}

	constexpr Vector2D operator -() const {
		return { -x, -y };
	}

	constexpr Vector2D operator +(const Vector2D &rhs) const {
		return { x + rhs.x, y + rhs.y };
	}

	constexpr Vector2D operator -(const Vector2D &rhs) const {
		return { x - rhs.x, y - rhs.y };
	}

	constexpr Vector2D operator *(double f) const {
		return { x * f, y * f };
	}

	constexpr Vector2D operator /(double f) const {
		return { x / f, y / f };
	}

	constexpr Vector2D &operator +=(const Vector2D &rhs) {
		return *this = *this + rhs;
	}

	constexpr Vector2D &operator -=(const Vector2D &rhs) {
		return *this = *this - rhs;
	}

	constexpr Vector2D &operator *=(const double rhs) {
		return *this = *this * rhs;
	}

	constexpr Vector2D &operator /=(const double rhs) {
		return *this = *this / rhs;
	}

	constexpr double cross(const Vector2D rhs) const {
		return x * rhs.y - rhs.x * y;
	}

	constexpr double dot(const Vector2D &rhs) const {
		return x * rhs.x + y * rhs.y;
	}

	constexpr double length_squared() const {
		return x * x + y * y;
	}

	constexpr double length() const {
		return std::sqrt(length_squared());
	}

	constexpr Vector2D normalize() const {
		return *this / length();
	}
};

constexpr Vector2D operator *(double lhs, const Vector2D &rhs) {
	return rhs * lhs;
}
