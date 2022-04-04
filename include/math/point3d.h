#pragma once

#include <initializer_list>
#include "math/vector3d.h"
#include "math/vector4d.h"

struct Point3D {
	double x, y, z;

	constexpr inline Point3D() : x(0), y(0), z(0) {}

	constexpr inline Point3D(double x, double y, double z) : x(x), y(y), z(z) {}

	constexpr inline Point3D(Vector3D v) : x(v.x), y(v.y), z(v.z) {}

	constexpr inline Point3D operator +(Vector3D v) const {
		return { x + v.x, y + v.y, z + v.z };
	}

	constexpr inline Point3D &operator +=(Vector3D v) {
		return *this = *this + v;
	}

	constexpr inline Point3D operator -(Vector3D v) const {
		return { x - v.x, y - v.y, z - v.z };
	}

	constexpr inline Point3D &operator -=(Vector3D v) {
		return *this = *this - v;
	}

	constexpr inline Vector3D operator -(Point3D rhs) const {
		return { x - rhs.x, y - rhs.y, z - rhs.z };
	}

	constexpr inline double distance_to_squared(Point3D rhs) const {
		return (*this - rhs).length_squared();
	}

	constexpr inline double distance_to(Point3D rhs) const {
		return (*this - rhs).length();
	}

	constexpr inline Point3D interpolate(Point3D to, double f) const {
		return {
			x * (1 - f) + to.x * f,
			y * (1 - f) + to.y * f,
			z * (1 - f) + to.z * f,
		};
	}

	template<typename C = std::initializer_list<Point3D>>
	constexpr inline static Point3D center(C points) {
		Vector3D s;
		for (auto p : points) {
			s += p.to_vector();
		}
		return Point3D(s / points.size());
	}

	constexpr inline Vector3D to_vector() const {
		return Vector3D(x, y, z);
	}
};

std::ostream &operator <<(std::ostream &o, const Point3D &m);
