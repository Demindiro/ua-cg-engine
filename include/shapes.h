#pragma once

#include <vector>
#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"
#include "lines.h"
#include "vector3d.h"

namespace shapes {
	struct Edge {
		unsigned int a, b;

		bool operator<(const Edge &rhs) const {
			auto ll = std::min(a, b), lh = std::max(a, b);
			auto rl = std::min(rhs.a, rhs.b), rh = std::max(rhs.a, rhs.b);
			return ll == rl ? lh < rh : ll < rl;
		}
	};

	struct Face {
		unsigned int a, b, c;
	};

	struct Color {
		double r, g, b;

		constexpr Color() : r(0), g(0), b(0) {}
		constexpr Color(double r, double g, double b) : r(r), g(g), b(b) {}

		constexpr Color operator +(Color rhs) const {
			return Color { r + rhs.r, g + rhs.g, b + rhs.b };
		}

		constexpr Color operator *(Color rhs) const {
			return Color { r * rhs.r, g * rhs.g, b * rhs.b };
		}

		constexpr void operator +=(Color rhs) {
			*this = *this + rhs;
		}

		constexpr void operator *=(Color rhs) {
			*this = *this * rhs;
		}

		constexpr Color clamp() const {
			return { ::clamp(r, 0.0, 1.0), ::clamp(g, 0.0, 1.0), ::clamp(b, 0.0, 1.0) };
		}

		inline img::Color to_img_color() const {
			auto c = clamp();
			return img::Color(round_up(c.r * 255), round_up(c.g * 255), round_up(c.b * 255));
		}
	};

	struct TriangleFigure {
		std::vector<Point3D> points;
		std::vector<Vector3D> normals;
		std::vector<Face> faces;
		Color ambient;
		Color diffuse;
		Color specular;
		double reflection;
	};

	struct FigureConfiguration {
		ini::Section &section;
		Matrix eye;
		bool with_lighting;
	};

	struct Lights {
		Color ambient;
	};

	Matrix transform_from_conf(const ini::Section &conf, const Matrix &projection);

	Color color_from_conf(const ini::Section &conf);

	void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, Point3D *points, unsigned int points_len, Edge *edges, unsigned int edges_len);

	inline void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, std::vector<Point3D> points, std::vector<Edge> edges) {
		platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	TriangleFigure platonic(const FigureConfiguration &conf, std::vector<Point3D> points, std::vector<Face> faces);

	img::EasyImage wireframe(const ini::Configuration &, bool with_z);

	img::EasyImage triangles(const ini::Configuration &, bool with_lighting);

	img::EasyImage draw(const std::vector<TriangleFigure> &figures, Lights *lights, unsigned int size, img::Color background);
}
