#pragma once

#include <cmath>

struct Vector3D {

	double x, y, z;

	constexpr Vector3D() : x(0), y(0), z(0) {}

	constexpr Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

	constexpr Vector3D operator +() const {
		return *this;
	}

	constexpr Vector3D operator -() const {
		return { -x, -y, -z };
	}

	constexpr Vector3D operator +(const Vector3D &rhs) const {
		return { x + rhs.x, y + rhs.y, z + rhs.z };
	}

	constexpr Vector3D operator -(const Vector3D &rhs) const {
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}

	constexpr Vector3D operator *(double f) const {
		return { x * f, y * f, z * f };
	}

	constexpr Vector3D operator /(double f) const {
		return { x / f, y / f, z / f };
	}

	constexpr Vector3D &operator +=(const Vector3D &rhs) {
		return *this = *this + rhs;
	}

	constexpr Vector3D &operator -=(const Vector3D &rhs) {
		return *this = *this - rhs;
	}

	constexpr Vector3D &operator *=(const double rhs) {
		return *this = *this * rhs;
	}

	constexpr Vector3D &operator /=(const double rhs) {
		return *this = *this / rhs;
	}

	constexpr Vector3D cross(const Vector3D rhs) const {
		return { y * rhs.z - rhs.y * z, rhs.x * z - x * rhs.z, x * rhs.y - rhs.x * y };
	}

	constexpr double dot(const Vector3D &rhs) const {
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	constexpr double length_squared() const {
		return x * x + y * y + z * z;
	}

	constexpr double length() const {
		return std::sqrt(length_squared());
	}

	constexpr Vector3D normalize() const {
		return *this / length();
	}
};

constexpr Vector3D operator *(double lhs, const Vector3D &rhs) {
	return rhs * lhs;
}
