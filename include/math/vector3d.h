#pragma once

#include <cmath>
#include <ostream>

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

	constexpr Vector3D operator *(Vector3D r) const {
		return { x * r.x, y * r.y, z * r.z };
	}

	constexpr Vector3D operator /(Vector3D r) const {
		return { x / r.x, y / r.y, z / r.z };
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
		return dot(*this);
	}

	constexpr double length() const {
		return std::sqrt(length_squared());
	}

	constexpr Vector3D normalize() const {
		return *this / length();
	}

	constexpr Vector3D max(Vector3D r) const {
		return { std::max(x, r.x), std::max(y, r.y), std::max(z, r.z) };
	}

	constexpr double max() const {
		return std::max(x, std::max(y, z));
	}

	constexpr Vector3D min(Vector3D r) const {
		return { std::min(x, r.x), std::min(y, r.y), std::min(z, r.z) };
	}

	constexpr double min() const {
		return std::min(x, std::min(y, z));
	}

	constexpr Vector3D abs() const {
		return { std::abs(x), std::abs(y), std::abs(z) };
	}

	constexpr Vector3D sign() const {
		return { std::copysign(1.0, x), std::copysign(1.0, y), std::copysign(1.0, z) };
	}
};

constexpr Vector3D operator *(double lhs, const Vector3D &rhs) {
	return rhs * lhs;
}

std::ostream &operator <<(std::ostream &o, const Vector3D &m);
