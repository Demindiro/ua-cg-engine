#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>
#include "math/point3d.h"
#include "math/vector2d.h"

namespace engine {

/**
 * \brief Depth buffer to figure out which pixels are closest to the camera.
 *
 * This buffer stores 3 values: The smallest 1/z (depth) value encountered and the
 * corresponding figure & triangle ID.
 *
 * If calculating the color value of a pixel is expensive (e.g. many point lights) then
 * the rendering should be done in two passes: update the ZBuffer and then use the figure
 * & triangle IDs to color the pixels. If it is very cheap to calculate the color however
 * (e.g. monotone wireframe) then a single pass should be performed using ZBuffer::replace().
 *
 * If both set() and replace() operations are used, then replace() operations should only occur
 * after all set() operations have been performed.
 */
class ZBuffer {
	std::vector<double> buffer;
	unsigned int width, height;

protected:
	double &operator()(unsigned int x, unsigned int y) {
		assert(x < width);
		assert(y < height);
		return buffer.at(x + y * width);
	}

	template<typename F>
	void triangle(
		Point3D a, Point3D b, Point3D c,
		double d, Vector2D offset,
		double bias,
		F callback
	);

public:
	ZBuffer() : width(0), height(0) {}

	ZBuffer(unsigned int width, unsigned int height) : width(width), height(height) {
		buffer.resize(width * height);
		clear();
	}

	double operator()(unsigned int x, unsigned int y) const {
		assert(x < width);
		assert(y < height);
		return buffer.at(x + y * width);
	}

	constexpr unsigned int get_width() const {
		return width;
	}

	constexpr unsigned int get_height() const {
		return height;
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

	/**
	 * \brief Place a triangle in the ZBuffer.
	 *
	 * \param d Scale factor.
	 */
	void triangle(Point3D a, Point3D b, Point3D c, double d, Vector2D offset, double bias);

	void clear() {
		for (auto &e : buffer) {
			e = std::numeric_limits<double>::infinity();
		}
	}
};

class TaggedZBuffer : public ZBuffer {
	std::vector<u_int16_t> figure_ids;
	std::vector<u_int32_t> triangle_ids;

public:
	struct IdPair {
		u_int16_t figure_id;
		u_int32_t triangle_id;
		double inv_z;

		constexpr bool is_valid() const {
			return figure_id != std::numeric_limits<u_int16_t>::max()
				&& triangle_id != std::numeric_limits<u_int32_t>::max();
		}

		constexpr bool operator ==(const IdPair &rhs) {
			return figure_id == rhs.figure_id && triangle_id == rhs.triangle_id;
		}

		constexpr bool operator !=(const IdPair &rhs) {
			return !(*this == rhs);
		}
	};

	TaggedZBuffer() : ZBuffer() {}

	TaggedZBuffer(unsigned int width, unsigned int height) : ZBuffer(width, height) {
		figure_ids.resize(width * height);
		triangle_ids.resize(width * height);
		clear();
	}
	
	/**
	 * \brief Set a figure & triangle ID at a pixel if the given 1/Z value is smaller.
	 */
	void set(unsigned int x, unsigned int y, IdPair pair) {
		bool lower = (*this)(x, y) > pair.inv_z;
		if (lower) {
			(*this)(x, y) = pair.inv_z;
			// Should be fine since the lengths of all buffers are equal
			figure_ids[x + y * get_width()] = pair.figure_id;
			triangle_ids[x + y * get_width()] = pair.triangle_id;
		}
	}

	/**
	 * \brief Get the figure & triangle ID at a pixel.
	 */
	IdPair get(unsigned int x, unsigned int y) {
		assert(x < get_width());
		assert(y < get_height());
		auto inv_z = (*this)(x, y); // Take advantage of bounds check.
		return {
			figure_ids[x + y * get_width()],
			triangle_ids[x + y * get_width()],
			inv_z
		};
	}

	/**
	 * \brief Place a triangle in the ZBuffer.
	 *
	 * \param d Scale factor.
	 * \param pair Pair of figure-triangle IDs. It's inv_z value is ignored.
	 */
	void triangle(Point3D a, Point3D b, Point3D c, double d, Vector2D offset, IdPair, double bias);

	void clear() {
		ZBuffer::clear();
		for (auto &e : figure_ids) {
			e = std::numeric_limits<u_int16_t>::max();
		}
		for (auto &e : triangle_ids) {
			e = std::numeric_limits<u_int32_t>::max();
		}
	}
};

}
