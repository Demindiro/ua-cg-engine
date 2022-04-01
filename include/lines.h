#pragma once

#include <vector>
#include "easy_image.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "render/color.h"
#include "render/rect.h"
#include "zbuffer.h"

namespace engine {

void calc_image_parameters(
	const render::Rect &bounds,
	uint size,
	double &d,
	Vector2D &offset,
	Vector2D &dimensions
);

img::EasyImage create_img(
	const render::Rect &bounds,
	uint size,
	const render::Color &background,
	double &d,
	Vector2D &offset
);

struct Line2D {
	Point2D a, b;
	render::Color color;

	Line2D(Point2D a, Point2D b, render::Color color) : a(a), b(b), color(color) {}

	void draw(img::EasyImage &) const;
};

class Lines2D {
	std::vector<Line2D> lines;

public:
	Lines2D() {}
	Lines2D(std::vector<Line2D> lines) : lines(lines) {}

	void add(Line2D);
	
	img::EasyImage draw(unsigned int size, render::Color background) const;
};

struct Line3D {
	Point3D a, b;
	render::Color color;

	Line3D(Point3D a, Point3D b, render::Color color) : a(a), b(b), color(color) {}

	void draw(img::EasyImage &, ZBuffer &z) const;
	void draw_clip(img::EasyImage &img, ZBuffer &z) const;
};

class Lines3D {
	std::vector<Line3D> lines;

public:
	Lines3D() {}
	Lines3D(std::vector<Line3D> lines) : lines(lines) {}

	void add(Line3D);
	
	img::EasyImage draw(unsigned int size, render::Color background, bool with_z) const;
};

}
