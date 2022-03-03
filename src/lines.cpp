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

inline void Line2D::draw(img::EasyImage &img) const {
	img.draw_line(round(a.x), round(a.y), round(b.x), round(b.y), color);
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
