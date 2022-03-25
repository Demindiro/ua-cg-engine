#include "lines.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "easy_image.h"
#include "engine.h"
#include "zbuffer.h"

using namespace std;

typedef unsigned int uint;

static img::EasyImage create_img(
	double min_x, double min_y,
	double max_x, double max_y,
	uint size,
	Color background,
	double &d,
	double &offset_x, double &offset_y
) {
	// Determine size, scale & offset
	double size_x = max_x - min_x, size_y = max_y - min_y;

	// Don't bother if the width or height is 0
	if (size_x == 0 || size_y == 0) {
		return img::EasyImage(0, 0);
	}

	double img_s = size / max(size_x, size_y);
	double img_x = size_x * img_s, img_y = size_y * img_s;

	d = img_x / size_x * 0.95;
	offset_x = (img_x - d * (min_x + max_x)) / 2;
	offset_y = (img_y - d * (min_y + max_y)) / 2;

	// Create image
	img::EasyImage img(round_up(img_x), round_up(img_y));
	img.clear(background);

	return img;
}

template<class T>
static img::EasyImage create_img(T &lines, uint size, Color background, double &d, double &offset_x, double &offset_y) {
	if (lines.empty()) {
		return img::EasyImage(0, 0);
	}

	// Determine bounds
	auto p = lines[0].a;
	double min_x = p.x, min_y = p.y, max_x = p.x, max_y = p.y;
	for (auto &l : lines) {
		min_x = min(min(min_x, l.a.x), l.b.x);
		max_x = max(max(max_x, l.a.x), l.b.x);
		min_y = min(min(min_y, l.a.y), l.b.y);
		max_y = max(max(max_y, l.a.y), l.b.y);
	}

	return create_img(min_x, min_y, max_x, max_y, size, background, d, offset_x, offset_y);
}

inline void Line2D::draw(img::EasyImage &img) const {
	img.draw_line(round_up(a.x), round_up(a.y), round_up(b.x), round_up(b.y), color);
}

void Lines2D::add(Line2D line) {
	this->lines.push_back(line);
}

img::EasyImage Lines2D::draw(uint size, Color background) const {
	double d, offset_x, offset_y;
	auto img = create_img(this->lines, size, background, d, offset_x, offset_y);

	// Transform & draw lines
	for (auto &l : this->lines) {
		Point2D a(l.a.x * d + offset_x, l.a.y * d + offset_y);
		Point2D b(l.b.x * d + offset_x, l.b.y * d + offset_y);
		Line2D(a, b, l.color).draw(img);
	}

	return img;
}

inline void Line3D::draw(img::EasyImage &img, ZBuffer &z) const {
	img.draw_zbuf_line(z, round_up(a.x), round_up(a.y), a.z, round_up(b.x), round_up(b.y), b.z, color);
}

void Lines3D::add(Line3D line) {
	this->lines.push_back(line);
}

img::EasyImage Lines3D::draw(uint size, Color background, bool with_z) const {
	// TODO wdym "unitialized", GCC?
	double d = NAN, offset_x = NAN, offset_y = NAN;
	auto img = create_img(this->lines, size, background, d, offset_x, offset_y);

	// Transform & draw lines
	if (with_z) {
		ZBuffer z(img.get_width(), img.get_height());
		for (auto &l : this->lines) {
			Point3D a(l.a.x * d + offset_x, l.a.y * d + offset_y, l.a.z);
			Point3D b(l.b.x * d + offset_x, l.b.y * d + offset_y, l.b.z);
			Line3D(a, b, l.color).draw(img, z);
		}
	} else {
		for (auto &l : this->lines) {
			Point2D a(l.a.x * d + offset_x, l.a.y * d + offset_y);
			Point2D b(l.b.x * d + offset_x, l.b.y * d + offset_y);
			Line2D(a, b, l.color).draw(img);
		}
	}

	return img;
}

void Triangles3D::add(Triangle3D triangle) {
	triangles.push_back(triangle);
}

img::EasyImage Triangles3D::draw(uint size, Color background) const {
	// TODO wdym "unitialized", GCC?
	double d = NAN, offset_x = NAN, offset_y = NAN;

	if (triangles.empty()) {
		return img::EasyImage(0, 0);
	}

	// Determine bounds
	auto p = triangles[0].a;
	double min_x, max_x, min_y, max_y;
	min_x = min_y = +numeric_limits<double>::infinity();
	max_x = max_y = -numeric_limits<double>::infinity();
	for (auto &t : triangles) {
		min_x = min(min(min_x, t.a.x / -t.a.z), min(t.b.x / -t.b.z, t.c.x / -t.c.z));
		min_y = min(min(min_y, t.a.y / -t.a.z), min(t.b.y / -t.b.z, t.c.y / -t.c.z));
		max_x = max(max(max_x, t.a.x / -t.a.z), max(t.b.x / -t.b.z, t.c.x / -t.c.z));
		max_y = max(max(max_y, t.a.y / -t.a.z), max(t.b.y / -t.b.z, t.c.y / -t.c.z));
	}

	auto img = create_img(min_x, min_y, max_x, max_y, size, background, d, offset_x, offset_y);

	// Transform & draw triangles
	ZBuffer z(img.get_width(), img.get_height());
	for (auto &t : triangles) {
		img.draw_zbuf_triag(z, t.a, t.b, t.c, d, offset_x, offset_y, t.color);
	}

	return img;
}
