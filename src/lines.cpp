#include "lines.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "easy_image.h"
#include "engine.h"
#include "render/color.h"
#include "render/rect.h"
#include "render/triangle.h"
#include "zbuffer.h"

#define MARGIN (0.95)

namespace engine {

using namespace std;
using namespace render;

typedef unsigned int uint;

void calc_image_parameters(
	const Rect &bounds,
	const uint px_size,
	double &d,
	Vector2D &offset,
	Vector2D &dimensions
) {
	auto size = bounds.size();

	// Don't bother if the width or height is 0
	if (size.x == 0 || size.y == 0) {
		dimensions.x = dimensions.y = 0;
		return;
	}

	dimensions = size * px_size / max(size.x, size.y);

	d = dimensions.x / size.x * MARGIN;
	offset = (dimensions - d * (bounds.min.to_vector() + bounds.max.to_vector())) / 2;
}

img::EasyImage create_img(
	const Rect &bounds,
	const uint px_size,
	const Color &background,
	double &d,
	Vector2D &offset
) {
	Vector2D img;
	calc_image_parameters(bounds, px_size, d, offset, img);
	return img::EasyImage(round_up(img.x), round_up(img.y), background.to_img_color());
}

void Line2D::draw(img::EasyImage &img) const {
	img.draw_line(round_up(a.x), round_up(a.y), round_up(b.x), round_up(b.y), color.to_img_color());
}

void Lines2D::add(Line2D line) {
	this->lines.push_back(line);
}

img::EasyImage Lines2D::draw(uint size, Color background) const {
	if (lines.empty()) {
		return img::EasyImage(0, 0);
	}

	// Determine bounds
	Rect rect;
	rect.min.x = rect.min.y = +numeric_limits<double>::infinity();
	rect.max.x = rect.max.y = -numeric_limits<double>::infinity();
	for (auto &l : lines) {
		rect = rect | l.a | l.b;
	}

	double d;
	Vector2D offset;
	auto img = create_img(rect, size, background, d, offset);

	// Transform & draw lines
	for (auto &l : this->lines) {
		Point2D a(l.a.x * d + offset.x, l.a.y * d + offset.y);
		Point2D b(l.b.x * d + offset.x, l.b.y * d + offset.y);
		Line2D(a, b, l.color).draw(img);
	}

	return img;
}

}
