#include "lines.h"
#include <algorithm>
#include <cmath>
#include "easy_image.h"

using namespace std;

typedef unsigned int uint;

// Returns -1 if lower than 0, otherwise 1.
static inline int signum_or_one(int x) {
	return x < 0 ? -1 : 1;
}

// FIXME there are still very slight differences with the reference images.
// Likely culprit is a difference in the implementation of this function.
void Line2D::draw(img::EasyImage &img) const {
	uint from_x = round(this->a.x);
	uint from_y = round(this->a.y);
	uint to_x   = round(this->b.x);
	uint to_y   = round(this->b.y);
	int dx = to_x - from_x, dy = to_y - from_y;
	if (abs(dx) < abs(dy)) {
		// Iterate over y
		auto s = signum_or_one(dy);
		auto f = (double)dx / dy * s;
		double fdx = this->a.x;
		for (unsigned int y = from_y; y != to_y; y += s) {
			img(round(fdx), y) = color;
			fdx += f;
		}
	} else {
		// Iterate over x
		auto s = signum_or_one(dx);
		auto f = (double)dy / dx * s;
		double fdy = this->a.y;
		for (unsigned int x = from_x; x != to_x; x += s) {
			img(x, round(fdy)) = color;
			fdy += f;
		}
	}
	img(to_x, to_y) = color;
}

void Lines2D::add(Line2D line) {
	this->lines.push_back(line);
}

img::EasyImage Lines2D::draw(uint size, Color background) const {
	if (lines.empty()) {
		return img::EasyImage(0, 0);
	}

	// Determine bounds
	auto p = this->lines[0].a;
	double min_x = p.x, min_y = p.y, max_x = p.x, max_y = p.y;
	for (auto &l : this->lines) {
		min_x = min(min(min_x, l.a.x), l.b.x);
		max_x = max(max(max_x, l.a.x), l.b.x);
		min_y = min(min(min_y, l.a.y), l.b.y);
		max_y = max(max(max_y, l.a.y), l.b.y);
	}

	// Determine size, scale & offset
	double size_x = max_x - min_x, size_y = max_y - min_y;

	double img_s = size / max(size_x, size_y);
	double img_x = size_x * img_s, img_y = size_y * img_s;

	double d = img_x / size_x * 0.95;
	double offset_x = (img_x - d * (min_x + max_x)) / 2;
	double offset_y = (img_y - d * (min_y + max_y)) / 2;

	// Don't bother if the width or height is 0
	if (size_x == 0 || size_y == 0) {
		return img::EasyImage(0, 0);
	}

	// Create image
	img::EasyImage img(round(img_x), round(img_y));
	img.clear(background);

	// Transform & draw lines
	for (auto &l : this->lines) {
		Point2D a(l.a.x * d + offset_x, l.a.y * d + offset_y);
		Point2D b(l.b.x * d + offset_x, l.b.y * d + offset_y);
		Line2D(a, b, l.color).draw(img);
	}

	return img;
}
