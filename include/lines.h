#pragma once

#include <vector>
#include "easy_image.h"
#include "zbuffer.h"

using Color = img::Color;

struct Point2D {
	double x, y;

	Point2D(double x, double y) : x(x), y(y) {}
};

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

struct Point3D {
	double x, y, z;

	Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
};

struct Line3D {
	Point3D a, b;
	Color color;

	Line3D(Point3D a, Point3D b, Color color) : a(a), b(b), color(color) {}

	void draw(img::EasyImage &, ZBuffer &z) const;
};

class Lines3D {
	std::vector<Line3D> lines;

public:
	Lines3D() {}
	Lines3D(std::vector<Line3D> lines) : lines(lines) {}

	void add(Line3D);
	
	img::EasyImage draw(unsigned int size, Color background, bool with_z) const;
};
