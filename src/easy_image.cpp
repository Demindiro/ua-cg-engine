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
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <cstring>
#include "engine.h"
#include "point3d.h"
#include "zbuffer.h"

#ifndef le32toh
#define le32toh(x) (x)
#endif

namespace {
// structs borrowed from wikipedia's article on the BMP file format
struct bmpfile_magic {
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
img::Color::Color() : blue(0), green(0), red(0) {}
img::Color::Color(uint8_t r, uint8_t g, uint8_t b)
	: blue(b), green(g), red(r) {}
img::Color::~Color() {}

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

img::EasyImage::EasyImage() : width(0), height(0), bitmap() {}

img::EasyImage::EasyImage(unsigned int _width, unsigned int _height,
						  Color color)
	: width(_width), height(_height), bitmap(width * height, color) {}

img::EasyImage::EasyImage(EasyImage const &img)
	: width(img.width), height(img.height), bitmap(img.bitmap) {}

img::EasyImage::~EasyImage() { bitmap.clear(); }

img::EasyImage &img::EasyImage::operator=(img::EasyImage const &img) {
	width = img.width;
	height = img.height;
	bitmap.assign(img.bitmap.begin(), img.bitmap.end());
	return (*this);
}

unsigned int img::EasyImage::get_width() const { return width; }

unsigned int img::EasyImage::get_height() const { return height; }

void img::EasyImage::clear(Color color) {
	for (std::vector<Color>::iterator i = bitmap.begin(); i != bitmap.end();
		 i++) {
		*i = color;
	}
}

img::Color &img::EasyImage::operator()(unsigned int x, unsigned int y) {
	assert(x < this->width);
	assert(y < this->height);
	return bitmap.at(x + y * width);
}

img::Color const &img::EasyImage::operator()(unsigned int x,
											 unsigned int y) const {
	assert(x < this->width);
	assert(y < this->height);
	return bitmap.at(x + y * width);
}

void img::EasyImage::draw_line(unsigned int x0, unsigned int y0,
							   unsigned int x1, unsigned int y1, Color color) {
	assert(x0 < this->width && y0 < this->height);
	assert(x1 < this->width && y1 < this->height);
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

void img::EasyImage::draw_zbuf_line(
	ZBuffer &zbuffer,
	unsigned int x0, unsigned int y0, double z0,
	unsigned int x1, unsigned int y1, double z1,
	Color color
) {
	assert(x0 < this->width && y0 < this->height);
	assert(x1 < this->width && y1 < this->height);
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

void img::EasyImage::draw_zbuf_triag(
	ZBuffer &zbuffer,
	Point3D a, Point3D b, Point3D c,
	double d,
	double dx, double dy,
	Color color
) {
	using namespace std;

	// Find repricoral Z-values first, which require unprojected points
	// Optimized version of 1 / (3 * a.z) + 1 / (3 * b.z) + 1 / (3 * c.z)
	// The former will emit 3 div instructions even with -Ofast
	double inv_g_z = (b.z * c.z + a.z * c.z + a.z * b.z) / (3 * a.z * b.z * c.z);
	double dzdx, dzdy;
	{
		auto u = Vector3D::vector(b.x - a.x, b.y - a.y, b.z - a.z);
		auto v = Vector3D::vector(c.x - a.x, c.y - a.y, c.z - a.z);
		auto w = u.cross(v);
		auto dk = d * w.dot(Vector3D::point(a.x, a.y, a.z));
		dzdx = -w.x / dk;
		dzdy = -w.y / dk;
	}
	
	// Project
	a.x = a.x * d / -a.z + dx, a.y = a.y * d / -a.z + dy;
	b.x = b.x * d / -b.z + dx, b.y = b.y * d / -b.z + dy;
	c.x = c.x * d / -c.z + dx, c.y = c.y * d / -c.z + dy;

	// These center coordaintes must be projected.
	double g_x = (a.x + b.x + c.x) / 3;
	double g_y = (a.y + b.y + c.y) / 3;

	// Y bounds
	double y_min = min(min(a.y, b.y), c.y);
	double y_max = max(max(a.y, b.y), c.y);
	unsigned int from_y = round_up(y_min + 0.5);
	unsigned int to_y   = round_up(y_max - 0.5);

	for (unsigned int y = from_y; y <= to_y; y++) {
		// Find intersections
		auto f = [y](Point3D &p, Point3D &q, double def = INFINITY) {
			return (y - p.y) * (y - q.y) <= 0
				? q.x + (p.x - q.x) * (y - q.y) / (p.y - q.y)
				: def;
		};
		auto g = [f](Point3D &p, Point3D &q) {
			return f(p, q, -INFINITY);
		};
		double xl = min(min(f(a, b), f(b, c)), f(c, a));
		double xr = max(max(g(a, b), g(b, c)), g(c, a));

		// X bounds
		double x_min = min(xl, xr);
		double x_max = max(xl, xr);
		unsigned int from_x = round_up(x_min + 0.5);
		unsigned int to_x   = round_up(x_max - 0.5);

		for (unsigned int x = from_x; x <= to_x; x++) {
			auto inv_z = 1.0001 * inv_g_z + (x - g_x) * dzdx + (y - g_y) * dzdy;
			if (zbuffer.replace(x, y, inv_z)) {
				(*this)(x, y) = color;
			}
		}
	}
}

std::ostream &img::operator<<(std::ostream &out, EasyImage const &image) {

	// temporaryily enable exceptions on output stream
	enable_exceptions(out, std::ios::badbit | std::ios::failbit);
	// declare some struct-vars we're going to need:
	bmpfile_magic magic;
	bmpfile_header file_header;
	bmp_header header;
	// calculate the total size of the pixel data
	unsigned int line_width = image.get_width() * 3; // 3 bytes per pixel
	unsigned int line_padding = 0;
	if (line_width % 4 != 0) {
		line_padding = 4 - (line_width % 4);
	}
	// lines must be aligned to a multiple of 4 bytes
	line_width += line_padding;
	unsigned int pixel_size = image.get_height() * line_width;

	// start filling the headers
	magic.magic[0] = 'B';
	magic.magic[1] = 'M';

	file_header.file_size = to_little_endian(pixel_size + sizeof(file_header) +
											 sizeof(header) + sizeof(magic));
	file_header.bmp_offset =
		to_little_endian(sizeof(file_header) + sizeof(header) + sizeof(magic));
	file_header.reserved_1 = 0;
	file_header.reserved_2 = 0;
	header.header_size = to_little_endian(sizeof(header));
	header.width = to_little_endian(image.get_width());
	header.height = to_little_endian(image.get_height());
	header.nplanes = to_little_endian(1);
	header.bits_per_pixel = to_little_endian(24); // 3bytes or 24 bits per pixel
	header.compress_type = 0;                     // no compression
	header.pixel_size = pixel_size;
	header.hres = to_little_endian(11811); // 11811 pixels/meter or 300dpi
	header.vres = to_little_endian(11811); // 11811 pixels/meter or 300dpi
	header.ncolors = 0;                    // no color palette
	header.nimpcolors = 0;                 // no important colors

	// okay that should be all the header stuff: let's write it to the stream
	out.write((char *)&magic, sizeof(magic));
	out.write((char *)&file_header, sizeof(file_header));
	out.write((char *)&header, sizeof(header));

	// okay let's write the pixels themselves:
	// they are arranged left->right, bottom->top, b,g,r

	// Manual buffering since ostream is horribly slow. (~50ms -> ~10ms = ~5x!)
	char buf[1 << 20]; // 1 MiB
	size_t buf_i = 0;

	for (unsigned int i = 0; i < image.get_height(); i++) {
		// loop over all lines
		for (unsigned int j = 0; j < image.get_width(); j++) {
			// loop over all pixels in a line
			// we cast &color to char*. since the color fields are ordered
			// blue,green,red they should be written automatically in the right
			// order
			if (buf_i >= sizeof(buf) - 4) {
				out.write(buf, buf_i);
				buf_i = 0;
			}
			memcpy(buf + buf_i, (char *)&image(j, i), 3 * sizeof(uint8_t));
			buf_i += 3 * sizeof(uint8_t);
		}
		if (buf_i >= sizeof(buf) - line_padding) {
			out.write(buf, buf_i);
			buf_i = 0;
		}
		memset(buf + buf_i, 0, line_padding);
		buf_i += line_padding;
	}
	out.write(buf, buf_i);

	// okay we should be done
	return out;
}
std::istream &img::operator>>(std::istream &in, EasyImage &image) {
	enable_exceptions(in, std::ios::badbit | std::ios::failbit);
	// declare some struct-vars we're going to need
	bmpfile_magic magic;
	bmpfile_header file_header;
	bmp_header header;
	// a temp buffer for reading the padding at the end of each line
	uint8_t padding[] = {0, 0, 0, 0};

	// read the headers && do some sanity checks
	in.read((char *)&magic, sizeof(magic));
	if (magic.magic[0] != 'B' || magic.magic[1] != 'M')
		throw UnsupportedFileTypeException(
			"Could not parse BMP File: invalid magic header");
	in.read((char *)&file_header, sizeof(file_header));
	in.read((char *)&header, sizeof(header));
	if (le32toh(header.pixel_size) + le32toh(file_header.bmp_offset) !=
		le32toh(file_header.file_size))
		throw UnsupportedFileTypeException(
			"Could not parse BMP File: file size mismatch");
	if (le32toh(header.header_size) != sizeof(header))
		throw UnsupportedFileTypeException(
			"Could not parse BMP File: Unsupported BITMAPV5HEADER size");
	if (le32toh(header.compress_type) != 0)
		throw UnsupportedFileTypeException(
			"Could not parse BMP File: Only uncompressed BMP files can be "
			"parsed");
	if (le32toh(header.nplanes) != 1)
		throw UnsupportedFileTypeException(
			"Could not parse BMP File: Only one plane should exist in the BMP "
			"file");
	if (le32toh(header.bits_per_pixel) != 24)
		throw UnsupportedFileTypeException(
			"Could not parse BMP File: Only 24bit/pixel BMP's are supported");
	// if height<0 -> read top to bottom instead of bottom to top
	bool invertedLines = from_little_endian(header.height) < 0;
	image.height = std::abs(from_little_endian(header.height));
	image.width = std::abs(from_little_endian(header.width));
	unsigned int line_padding =
		from_little_endian(header.pixel_size) / image.height -
		(3 * image.width);
	// re-initialize the image bitmap
	image.bitmap.clear();
	image.bitmap.assign(image.height * image.width, Color());
	// okay let's read the pixels themselves:
	// they are arranged left->right., bottom->top if height>0, top->bottom if
	// height<0, b,g,r
	for (unsigned int i = 0; i < image.get_height(); i++) {
		// loop over all lines
		for (unsigned int j = 0; j < image.get_width(); j++) {
			// loop over all pixels in a line
			// we cast &color to char*. since the color fields are ordered
			// blue,green,red, the data read should be written in the right
			// variables
			if (invertedLines) {
				// store top-to-bottom
				in.read((char *)&image(j, image.height - 1 - i),
						3 * sizeof(uint8_t));
			} else {
				// store bottom-to-top
				in.read((char *)&image(j, i), 3 * sizeof(uint8_t));
			}
		}
		if (line_padding > 0) {
			in.read((char *)padding, line_padding);
		}
	}
	// okay we're done
	return in;
}
