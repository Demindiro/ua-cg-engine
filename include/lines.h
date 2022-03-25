#pragma once

#include <vector>
#include "easy_image.h"
#include "point3d.h"
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

struct Triangle3D {
	Point3D a, b, c;
	Color color;

	Triangle3D(Point3D a, Point3D b, Point3D c, Color color) : a(a), b(b), c(c), color(color) {}
};

class Triangles3D {
	std::vector<Triangle3D> triangles;

public:
	Triangles3D() {}
	Triangles3D(std::vector<Triangle3D> triangles) : triangles(triangles) {}

	void add(Triangle3D);

	img::EasyImage draw(unsigned int size, Color background) const;
};
