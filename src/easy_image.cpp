/*
 * easy_image.cc
 * Copyright (C) 2011  Daniel van den Akker
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "easy_image.h"
#include <cstdlib>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <cstring>
#include "engine.h"
#include "math/point3d.h"
#include "zbuffer.h"

#ifndef le32toh
#define le32toh(x) (x)
#endif

using namespace engine;

namespace {
// structs borrowed from wikipedia's article on the BMP file format
struct bmpfile_magic {
	uint8_t padding[2];
	uint8_t magic[2];
};

struct bmpfile_header {
	uint32_t file_size;
	uint16_t reserved_1;
	uint16_t reserved_2;
	uint32_t bmp_offset;
};
struct bmp_header {
	uint32_t header_size;
	int32_t width;
	int32_t height;
	uint16_t nplanes;
	uint16_t bits_per_pixel;
	uint32_t compress_type;
	uint32_t pixel_size;
	int32_t hres;
	int32_t vres;
	uint32_t ncolors;
	uint32_t nimpcolors;
};

static size_t calc_meta_size() {
	return sizeof(bmpfile_magic)
		+ sizeof(bmpfile_header)
		+ sizeof(bmp_header);
}

static size_t calc_size(unsigned int width, unsigned int height) {
	assert((unsigned long long)width * height == width * height && "size overflow");
	return calc_meta_size() + ((width * 3 + 3) & ~3L) * height;
}

// copy-pasted from lparser.cc to allow these classes to be used independently
// from each other
class enable_exceptions {
	private:
	std::ios &ios;
	std::ios::iostate state;

	public:
	enable_exceptions(std::ios &an_ios, std::ios::iostate exceptions)
		: ios(an_ios) {
		state = ios.exceptions();
		ios.exceptions(exceptions);
	}
	~enable_exceptions() { ios.exceptions(state); }
};
// helper function to convert a number (char, int, ...) to little endian
// regardless of the endiannes of the system
// more efficient machine-dependent functions exist, but this one is more
// portable
template <typename T> T to_little_endian(T value) {
	// yes, unions must be used with caution, but this is a case in which a
	// union is needed
	union {
		T t;
		uint8_t bytes[sizeof(T)];
	} temp_storage;

	for (uint8_t i = 0; i < sizeof(T); i++) {
		temp_storage.bytes[i] = value & 0xFF;
		value >>= 8;
	}
	return temp_storage.t;
}

/*
static uint16_t htole16(uint16_t t) {
	return to_little_endian(t);
}

static uint32_t htole32(uint32_t t) {
	return to_little_endian(t);
}
*/

template <typename T> T from_little_endian(T value) {
	// yes, unions must be used with caution, but this is a case in which a
	// union is needed
	union {
		T t;
		uint8_t bytes[sizeof(T)];
	} temp_storage;
	temp_storage.t = value;
	T retVal = 0;

	for (uint8_t i = 0; i < sizeof(T); i++) {
		retVal = (retVal << 8) | temp_storage.bytes[sizeof(T) - i - 1];
	}
	return retVal;
}

} // namespace

img::UnsupportedFileTypeException::UnsupportedFileTypeException(
	std::string const &msg)
	: message(msg) {}
img::UnsupportedFileTypeException::UnsupportedFileTypeException(
	const UnsupportedFileTypeException &original)
	: std::exception(original), message(original.message) {}
img::UnsupportedFileTypeException::~UnsupportedFileTypeException() throw() {}
img::UnsupportedFileTypeException &img::UnsupportedFileTypeException::operator=(
	UnsupportedFileTypeException const &original) {
	this->message = original.message;
	return *this;
}
const char *img::UnsupportedFileTypeException::what() const throw() {
	return message.c_str();
}

img::EasyImage::EasyImage() : img::EasyImage::EasyImage(0, 0) {}

img::EasyImage::EasyImage(unsigned int width, unsigned int height, Color color) {
	// FIXME some bytes are unitialized. This isn't too bad right now but may leak secrets
	// if this code is used elsewhere.
	data = malloc(calc_size(width, height));
	if (data == NULL)
		throw std::bad_alloc();
	row_size = (width * 3 + 3) & ~3; // Round up to multiple of 4

	bmpfile_magic *fm = (bmpfile_magic *)data;
	bmpfile_header *fh = (bmpfile_header *)((char *)data + sizeof(*fm));
	bmp_header *h = (bmp_header *)((char *)data + sizeof(*fm) + sizeof(*fh));

	fm->padding[0] = 0;
	fm->padding[1] = 0;
	fm->magic[0] = 'B';
	fm->magic[1] = 'M';

	fh->file_size = htole32(calc_size(width, height));
	fh->bmp_offset = htole32(calc_meta_size() - 2);

	h->header_size = htole32(sizeof(*h));
	h->width = htole32(width);
	h->height = htole32(height);
	h->nplanes = htole16(1);
	h->bits_per_pixel = htole16(24); // 3bytes or 24 bits per pixel
	h->compress_type = 0;                     // no compression
	h->pixel_size = htole32(row_size * height);
	h->hres = htole32(11811); // 11811 pixels/meter or 300dpi
	h->vres = htole32(11811); // 11811 pixels/meter or 300dpi
	h->ncolors = 0;                    // no color palette
	h->nimpcolors = 0;                 // no important colors

	clear(color);
}

unsigned int img::EasyImage::get_width() const {
	assert(data != nullptr && "Image was moved");
	return le32toh(((bmp_header *)((char *)data + sizeof(bmpfile_magic) + sizeof(bmpfile_header)))->width);
}

unsigned int img::EasyImage::get_height() const {
	assert(data != nullptr && "Image was moved");
	return le32toh(((bmp_header *)((char *)data + sizeof(bmpfile_magic) + sizeof(bmpfile_header)))->height);
}

void img::EasyImage::clear(Color color) {
	for (unsigned int y = 0; y < get_height(); y++) {
		for (unsigned int x = 0; x < get_width(); x++) {
			(*this)(x, y) = color;
		}
	}
}

img::Color &img::EasyImage::operator()(unsigned int x, unsigned int y) {
	assert(x < get_width());
	assert(y < get_height());
	return ((Color *)((char *)data + calc_meta_size() + (size_t)row_size * y))[x];
}

img::Color const &img::EasyImage::operator()(unsigned int x, unsigned int y) const {
	assert(x < get_width());
	assert(y < get_height());
	return ((Color *)((char *)data + calc_meta_size() + (size_t)row_size * y))[x];
}

void img::EasyImage::draw_line(unsigned int x0, unsigned int y0,
							   unsigned int x1, unsigned int y1, Color color) {
	assert(x0 < get_width() && y0 < get_height());
	assert(x1 < get_width() && y1 < get_height());
	if (x0 == x1) {
		// special case for x0 == x1
		for (unsigned int i = std::min(y0, y1); i <= std::max(y0, y1); i++) {
			(*this)(x0, i) = color;
		}
	} else if (y0 == y1) {
		// special case for y0 == y1
		for (unsigned int i = std::min(x0, x1); i <= std::max(x0, x1); i++) {
			(*this)(i, y0) = color;
		}
	} else {
		if (x0 > x1) {
			// flip points if x1>x0: we want x0 to have the lowest value
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		double m = ((double)y1 - (double)y0) / ((double)x1 - (double)x0);
		if (-1.0 <= m && m <= 1.0) {
			for (unsigned int i = 0; i <= (x1 - x0); i++) {
				(*this)(x0 + i, (unsigned int)round_up(y0 + m * i)) = color;
			}
		} else if (m > 1.0) {
			for (unsigned int i = 0; i <= (y1 - y0); i++) {
				(*this)((unsigned int)round_up(x0 + (i / m)), y0 + i) = color;
			}
		} else if (m < -1.0) {
			for (unsigned int i = 0; i <= (y0 - y1); i++) {
				(*this)((unsigned int)round_up(x0 - (i / m)), y0 - i) = color;
			}
		}
	}
}

static bool clip(
	double &x0, double &y0, double z0,
	double x1, double y1, double z1,
	unsigned int w, unsigned int h
) {
	w--, h--;

	auto p = 0.0;
	if (x0 < 0) {
		p = -x0 / (-x0 + x1);
	} else if (x0 > w) {
		p = (x0 - w) / ((x0 - w) + (w - x1));
	}
	p = std::clamp(p, 0.0, 1.0);
	x0 = x0 * (1 - p) + x1 * p;
	y0 = y0 * (1 - p) + y1 * p;
	z0 = z0 * (1 - p) + z1 * p;

	p = 0.0;
	if (y0 < 0) {
		p = -y0 / (-y0 + y1);
	} else if (y0 > h) {
		p = (y0 - h) / ((y0 - h) + (h - y1));
	}
	p = std::clamp(p, 0.0, 1.0);
	x0 = x0 * (1 - p) + x1 * p;
	y0 = y0 * (1 - p) + y1 * p;
	z0 = z0 * (1 - p) + z1 * p;

	auto inside = !((x0 < 0 && x1 < 0) || (y0 < 0 && y1 < 0) || (x0 > w && x1 > w) || (y0 > h && y1 > h));
	return inside;
}

void img::EasyImage::draw_line_clip(double x0, double y0, double x1, double y1, Color color) {
	double z0 = 0, z1 = 0;
	if (!clip(x0, y0, z0, x1, y1, z1, get_width(), get_height()))
		return;
	if (!clip(x1, y1, z1, x0, y0, z0, get_width(), get_height()))
		return;
	draw_line(
		round_up(x0),
		round_up(y0),
		round_up(x1),
		round_up(y1),
		color
	);
}

void img::EasyImage::draw_zbuf_line(
	ZBuffer &zbuffer,
	unsigned int x0, unsigned int y0, double z0,
	unsigned int x1, unsigned int y1, double z1,
	Color color
) {
	assert(x0 < get_width() && y0 < get_height());
	assert(x1 < get_width() && y1 < get_height());
	double inv_z0 = 1 / z0, inv_z1 = 1 / z1;
	double a = NAN;
	auto set = [&](unsigned int x, unsigned int y, unsigned int i) {
		double p = i / a;
		// We always start from p0
		double inv_z = p * inv_z0 + (1 - p) * inv_z1;
		if (zbuffer.replace(x, y, inv_z)) {
			(*this)(x, y) = color;
		}
	};

	if (x0 == x1) {
		// special case for x0 == x1
		a = std::max(y0, y1);
		for (unsigned int i = std::min(y0, y1); i <= std::max(y0, y1); i++) {
			set(x0, i, i);
		}
	} else if (y0 == y1) {
		// special case for y0 == y1
		a = std::max(x0, x1);
		for (unsigned int i = std::min(x0, x1); i <= std::max(x0, x1); i++) {
			set(i, y0, i);
		}
	} else {
		if (x0 > x1) {
			// flip points if x1>x0: we want x0 to have the lowest value
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		double m = ((double)y1 - (double)y0) / ((double)x1 - (double)x0);
		if (-1.0 <= m && m <= 1.0) {
			a = x1 - x0;
			for (unsigned int i = 0; i <= (x1 - x0); i++) {
				set(x0 + i, (unsigned int)round_up(y0 + m * i), i);
			}
		} else if (m > 1.0) {
			a = y1 - y0;
			for (unsigned int i = 0; i <= (y1 - y0); i++) {
				set((unsigned int)round_up(x0 + (i / m)), y0 + i, i);
			}
		} else if (m < -1.0) {
			a = y0 - y1;
			for (unsigned int i = 0; i <= (y0 - y1); i++) {
				set((unsigned int)round_up(x0 - (i / m)), y0 - i, i);
			}
		}
	}
}

void img::EasyImage::draw_zbuf_line_clip(
	ZBuffer &zbuffer,
	double x0, double y0, double z0,
	double x1, double y1, double z1,
	Color color
) {
	if (!clip(x0, y0, z0, x1, y1, z1, get_width(), get_height()))
		return;
	if (!clip(x1, y1, z1, x0, y0, z0, get_width(), get_height()))
		return;
	draw_zbuf_line(
		zbuffer,
		round_up(x0),
		round_up(y0),
		z0,
		round_up(x1),
		round_up(y1),
		z1,
		color
	);
}

std::ostream &img::operator<<(std::ostream &out, EasyImage const &image) {
	enable_exceptions(out, std::ios::badbit | std::ios::failbit);
	out.write((char *)image.data + 2, calc_size(image.get_width(), image.get_height()) - 2);
	return out;
}
std::istream &img::operator>>(std::istream &in, EasyImage &image) {
	// FIXME sanitize input

	enable_exceptions(in, std::ios::badbit | std::ios::failbit);

	in.seekg(0, std::ios::end);
	size_t size = in.tellg();
	void *d = malloc(2 + size);
	if (d == NULL)
		throw std::bad_alloc();
	in.seekg(0, std::ios::beg);

	free((char *)image.data);
	in.read((char *)d + 2, size);
	memset(d, 0, 2);
	image.data = (char *)d;
	image.row_size = (image.get_width() * 3 + 3) & ~3; // Round up to multiple of 4

	return in;
}

std::ostream &img::operator<<(std::ostream &o, const img::Color &c) {
	return o << "(" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ")";
}
