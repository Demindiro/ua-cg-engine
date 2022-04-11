#pragma once

#include <cassert>
#include <ostream>
#include "math/point2d.h"
#include "math/vector2d.h"

struct Matrix2D {
	Vector2D columns[2];

	constexpr Matrix2D(Vector2D a, Vector2D b) {
		columns[0] = a;
		columns[1] = b;
	}

	constexpr Matrix2D() {
		columns[0] = { 1, 0 };
		columns[1] = { 0, 1 };
	}

	constexpr const Vector2D &operator [](unsigned int col) {
		assert(col < 2);
		return columns[col];
	}

	constexpr const Vector2D &operator [](unsigned int col) const {
		assert(col < 2);
		return columns[col];
	}

	constexpr double operator ()(unsigned int row, unsigned int col) const {
		return columns[col][row];
	}

	constexpr double &operator ()(unsigned int row, unsigned int col) {
		return columns[col][row];
	}

	constexpr Matrix2D operator *(const Matrix2D &rhs) const {
		auto l = transpose();
		auto &r = rhs;
		return {
			{ l[0].dot(r[0]), l[1].dot(r[0]) },
			{ l[0].dot(r[1]), l[1].dot(r[1]) },
		};
	}

	constexpr Matrix2D operator *(double f) const {
		return { columns[0] * f, columns[1] * f };
	}

	constexpr Matrix2D operator /(double f) const {
		return { columns[0] / f, columns[1] / f };
	}

	constexpr Matrix2D &operator *=(const Matrix2D &rhs) {
		return *this = *this * rhs;
	}

	constexpr Matrix2D transpose() const {
		auto &m = *this;
		return {
			{ m[0].x, m[1].x },
			{ m[0].y, m[1].y },
		};
	}

	constexpr double determinant() const {
		auto &m = *this;
		return m[0].x * m[1].y - m[0].y * m[1].x;
	}

	constexpr Matrix2D inv() const {
		auto a = columns[0].x, b = columns[1].x;
		auto c = columns[0].y, d = columns[1].y;
		return Matrix2D {
			{ d, -c },
			{ -b, a },
		} / determinant();
	}
};

constexpr Vector2D operator *(const Vector2D &v, const Matrix2D &m) {
	return { v.dot(m[0]), v.dot(m[1]) };
}

constexpr Vector2D operator *=(Vector2D &v, const Matrix2D &m) {
	return v = v * m;
}

constexpr Point2D operator *(const Point2D &v, const Matrix2D &m) {
	auto r = Vector2D(v.x, v.y) * m;
	return { r.x, r.y };
}

constexpr Point2D operator *=(Point2D &v, const Matrix2D &m) {
	return v = v * m;
}

std::ostream &operator <<(std::ostream &, const Matrix2D &);
