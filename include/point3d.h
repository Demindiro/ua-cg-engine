#pragma once

#include <initializer_list>
#include "vector3d.h"

struct Point3D {
	double x, y, z;

	constexpr inline Point3D() : x(0), y(0), z(0) {}
	constexpr inline Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
	inline Point3D(Vector3D v) : x(v.x), y(v.y), z(v.z) {}

	inline Point3D operator *(const Matrix &m) const {
		auto v = Vector3D::point(this->x, this->y, this->z) * m;
		return { v.x, v.y, v.z };
	}

	inline Vector3D operator -(Point3D rhs) const {
		return Vector3D::vector(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	}

	inline Point3D operator +(Vector3D v) const {
		return { this->x + v.x, this->y + v.y, this->z + v.z };
	}

	inline Point3D operator -(Vector3D v) const {
		return { this->x - v.x, this->y - v.y, this->z - v.z };
	}

	inline void operator *=(const Matrix &m) {
		*this = *this * m;
	}

	constexpr inline Point3D interpolate(Point3D to, double f) const {
		return {
			this->x * (1 - f) + to.x * f,
			this->y * (1 - f) + to.y * f,
			this->z * (1 - f) + to.z * f,
		};
	}

	constexpr inline static Point3D center(std::initializer_list<Point3D> points) {
		double x = 0, y = 0, z = 0;
		for (auto p : points) {
			x += p.x;
			y += p.y;
			z += p.z;
		}
		return { x / points.size(), y / points.size(), z / points.size() };
	}

	inline Vector3D to_vector() const {
		return Vector3D::point(this->x, this->y, this->z);
	}
};
