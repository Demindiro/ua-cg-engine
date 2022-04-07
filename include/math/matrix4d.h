#pragma once

#include <cassert>
#include <ostream>
#include "math/point3d.h"
#include "math/vector3d.h"
#include "math/vector4d.h"

struct Matrix4D {
	Vector4D columns[4];

	constexpr Matrix4D(Vector4D a, Vector4D b, Vector4D c, Vector4D d) {
		columns[0] = a;
		columns[1] = b;
		columns[2] = c;
		columns[3] = d;
	}

	constexpr Matrix4D() {
		columns[0] = { 1, 0, 0, 0 };
		columns[1] = { 0, 1, 0, 0 };
		columns[2] = { 0, 0, 1, 0 };
		columns[3] = { 0, 0, 0, 1 };
	}

	// TODO rename to something more sensible
	constexpr Vector4D x() const {
		return { (*this)[0].x, (*this)[1].x, (*this)[2].x, (*this)[3].x };
	}

	constexpr Vector4D y() const {
		return { (*this)[0].y, (*this)[1].y, (*this)[2].y, (*this)[3].y };
	}

	constexpr Vector4D z() const {
		return { (*this)[0].z, (*this)[1].z, (*this)[2].z, (*this)[3].z };
	}

	constexpr Vector4D w() const {
		return { (*this)[0].w, (*this)[1].w, (*this)[2].w, (*this)[3].w };
	}

	constexpr const Vector4D &operator [](unsigned int col) {
		assert(col < 4);
		return columns[col];
	}

	constexpr const Vector4D &operator [](unsigned int col) const {
		assert(col < 4);
		return columns[col];
	}

	// FIXME use 0-based indexing
	constexpr double operator ()(unsigned int row, unsigned int col) const {
		return columns[col - 1][row - 1];
	}

	constexpr double &operator ()(unsigned int row, unsigned int col) {
		return columns[col - 1][row - 1];
	}

	constexpr Matrix4D operator *(const Matrix4D &rhs) {
		auto l = transpose();
		auto &r = rhs;
		return {
			{ l[0].dot(r[0]), l[1].dot(r[0]), l[2].dot(r[0]), l[3].dot(r[0]) },
			{ l[0].dot(r[1]), l[1].dot(r[1]), l[2].dot(r[1]), l[3].dot(r[1]) },
			{ l[0].dot(r[2]), l[1].dot(r[2]), l[2].dot(r[2]), l[3].dot(r[2]) },
			{ l[0].dot(r[3]), l[1].dot(r[3]), l[2].dot(r[3]), l[3].dot(r[3]) },
		};
	}

	constexpr Matrix4D &operator *=(const Matrix4D &rhs) {
		return *this = *this * rhs;
	}

	constexpr Matrix4D transpose() const {
		auto &m = *this;
		return {
			{ m[0].x, m[1].x, m[2].x, m[3].x },
			{ m[0].y, m[1].y, m[2].y, m[3].y },
			{ m[0].z, m[1].z, m[2].z, m[3].z },
			{ m[0].w, m[1].w, m[2].w, m[3].w },
		};
	}
};

constexpr Vector4D operator *(const Vector4D &v, const Matrix4D &m) {
	return { v.dot(m[0]), v.dot(m[1]), v.dot(m[2]), v.dot(m[3]) };
}

constexpr Vector4D operator *=(Vector4D &v, const Matrix4D &m) {
	return v = v * m;
}

constexpr Point3D operator *(const Point3D &v, const Matrix4D &m) {
	auto r = Vector4D(v.x, v.y, v.z, 1) * m;
	return { r.x, r.y, r.z };
}

constexpr Point3D operator *=(Point3D &v, const Matrix4D &m) {
	return v = v * m;
}

constexpr Vector3D operator *(const Vector3D &v, const Matrix4D &m) {
	auto r = Vector4D(v.x, v.y, v.z, 0) * m;
	return { r.x, r.y, r.z };
}

constexpr Vector3D operator *=(Vector3D &v, const Matrix4D &m) {
	return v = v * m;
}

std::ostream &operator <<(std::ostream &, const Matrix4D &);
