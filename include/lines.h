#pragma once

#include <vector>
#include "easy_image.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "zbuffer.h"

using Color = img::Color;

void calc_image_parameters(
	double min_x, double min_y,
	double max_x, double max_y,
	uint size,
	double &d,
	double &offset_x, double &offset_y,
	double &img_x, double &img_y
);

img::EasyImage create_img(
	double min_x, double min_y,
	double max_x, double max_y,
	uint size,
	Color background,
	double &d,
	double &offset_x, double &offset_y
);

struct Line2D {
	Point2D a, b;
	Color color;

	Line2D(Point2D a, Point2D b, Color color) : a(a), b(b), color(color) {}

	void draw(img::EasyImage &) const;
};

class Lines2D {
	std::vector<Line2D> lines;

public:
	Lines2D() {}
	Lines2D(std::vector<Line2D> lines) : lines(lines) {}

	void add(Line2D);
	
	img::EasyImage draw(unsigned int size, Color background) const;
};

struct Line3D {
	Point3D a, b;
	Color color;

	Line3D(Point3D a, Point3D b, Color color) : a(a), b(b), color(color) {}

	void draw(img::EasyImage &, ZBuffer &z) const;
	void draw_clip(img::EasyImage &img, ZBuffer &z) const;
};

class Lines3D {
	std::vector<Line3D> lines;

public:
	Lines3D() {}
	Lines3D(std::vector<Line3D> lines) : lines(lines) {}

	void add(Line3D);
	
	img::EasyImage draw(unsigned int size, Color background, bool with_z) const;
};
