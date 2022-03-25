#pragma once

#include <cassert>
#include <cmath>
#include <vector>

class ZBuffer {
	std::vector<double> buffer;
	unsigned int width, height;

	double operator()(unsigned int x, unsigned int y) const {
		assert(x < width);
		assert(y < height);
		return buffer.at(x + y * width);
	}

	double &operator()(unsigned int x, unsigned int y) {
		assert(x < width);
		assert(y < height);
		return buffer.at(x + y * width);
	}

public:
	ZBuffer(unsigned int width, unsigned int height) : width(width), height(height) {
		buffer.resize(width * height);
		for (auto &c : buffer)
			c = INFINITY;
	}

	/**
	 * \brief Replace a 1/Z value with a *lower* value.
	 *
	 * \return true if the given value is lower, false otherwise
	 */
	bool replace(unsigned int x, unsigned int y, double inv_z) {
		bool lower = (*this)(x, y) > inv_z;
		(*this)(x, y) = lower ? inv_z : (*this)(x, y);
		return lower;
	}
};
