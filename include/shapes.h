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

		constexpr Color operator *(double f) const {
			return Color { r * f, g * f, b * f };
		}

		constexpr Color operator /(double f) const {
			return Color { r / f, g / f, b / f };
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
		/**
		 * \brief Normals for calculating lighting. Empty if no lighting.
		 */
		std::vector<Vector3D> normals;
		std::vector<Face> faces;
		Color ambient;
		Color diffuse;
		Color specular;
		double reflection;
		unsigned int reflection_int; // Not 0 if defined
		/**
		 * \brief Whether each normal is part of a face or a point.
		 */
		bool face_normals;
		bool can_cull;
		bool clipped;
	};

	/**
	 * TriangleFigure optimized for ZBuffer use only (e.g. shadows).
	 */
	struct ZBufferTriangleFigure {
		std::vector<Point3D> points;
		std::vector<Face> faces;
		bool can_cull;

		ZBufferTriangleFigure(bool can_cull)
			: can_cull(can_cull)
		{}

		ZBufferTriangleFigure(const TriangleFigure &fig)
			: points(fig.points), faces(fig.faces), can_cull(fig.can_cull)
		{}

		ZBufferTriangleFigure(const TriangleFigure &fig, const Matrix &mat)
			: faces(fig.faces), can_cull(fig.can_cull)
		{
			points.reserve(fig.points.size());
			for (auto &p : fig.points) {
				points.push_back(p * mat);
			}
		}

		static std::vector<ZBufferTriangleFigure> convert(const std::vector<TriangleFigure> &figs) {
			std::vector<ZBufferTriangleFigure> zfigs;
			zfigs.reserve(figs.size());
			for (auto &f : figs) {
				zfigs.push_back(ZBufferTriangleFigure(f));
			}
			return zfigs;
		}

		static std::vector<ZBufferTriangleFigure> convert(const std::vector<TriangleFigure> &figs, const Matrix &mat) {
			std::vector<ZBufferTriangleFigure> zfigs;
			zfigs.reserve(figs.size());
			for (auto &f : figs) {
				zfigs.push_back(ZBufferTriangleFigure(f, mat));
			}
			return zfigs;
		}
	};

	struct FigureConfiguration {
		ini::Section &section;
		Matrix eye;
		bool with_lighting;
		bool face_normals;
	};

	struct DirectionalLight {
		Vector3D direction;
		Color diffuse, specular;
	};

	struct PointLight {
		Point3D point;
		Color diffuse, specular;
		double spot_angle_cos;
		struct {
			Matrix eye;
			ZBuffer zbuf;
			double d, dx, dy;
		} cached;
	};

	struct Lights {
		std::vector<DirectionalLight> directional;
		std::vector<PointLight> point;
		std::vector<ZBufferTriangleFigure> zfigures;
		Matrix eye;
		unsigned int shadow_mask;
		Color ambient;
		bool shadows;
	};

	Matrix transform_from_conf(const ini::Section &conf, const Matrix &projection);

	Color color_from_conf(const ini::Section &conf);

	/**
	 * \brief Generic face normal calculator.
	 */
	std::vector<Vector3D> calculate_face_normals(const std::vector<Point3D> &points, const std::vector<Face> &faces);

	void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, Point3D *points, unsigned int points_len, Edge *edges, unsigned int edges_len);

	inline void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, std::vector<Point3D> points, std::vector<Edge> edges) {
		platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
	}

	TriangleFigure platonic(const FigureConfiguration &conf, std::vector<Point3D> points, std::vector<Face> faces);

	img::EasyImage wireframe(const ini::Configuration &, bool with_z);

	img::EasyImage triangles(const ini::Configuration &, bool with_lighting);

	img::EasyImage draw(std::vector<TriangleFigure> figures, Lights lights, unsigned int size, img::Color background);
}
