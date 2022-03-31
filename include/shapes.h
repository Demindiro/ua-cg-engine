#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <ostream>
#include <vector>
#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"
#include "lines.h"
#include "math/matrix4d.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/vector3d.h"

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
		Color(img::Color c) : r(c.red / 255.0), g(c.green / 255.0), b(c.blue / 255.0) {}

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
			return { std::clamp(r, 0.0, 1.0), std::clamp(g, 0.0, 1.0), std::clamp(b, 0.0, 1.0) };
		}

		inline img::Color to_img_color() const {
			auto c = clamp();
			return img::Color(round_up(c.r * 255), round_up(c.g * 255), round_up(c.b * 255));
		}
	};

	struct Texture {
		img::EasyImage image;

		img::Color get_clamped(Point2D uv) {
			unsigned int u = round_up((image.get_width() - 1) * std::clamp(uv.x, 0.0, 1.0));
			unsigned int v = round_up((image.get_height() - 1) * std::clamp(uv.y, 0.0, 1.0));
			return image(u, v);
		}
	};

	enum TextureMapping {
		FLAT,
	};

	struct TriangleFigure {
		std::vector<Point3D> points;
		std::vector<Point2D> uv;
		/**
		 * \brief Normals for calculating lighting. Empty if no lighting.
		 */
		std::vector<Vector3D> normals;
		std::vector<Face> faces;
		std::optional<Texture> texture;
		Color ambient;
		Color diffuse;
		Color specular;
		double reflection;
		unsigned int reflection_int; // Not 0 if defined
		TextureMapping texture_mapping;
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

		ZBufferTriangleFigure(const TriangleFigure &fig, const Matrix4D &mat)
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

		static std::vector<ZBufferTriangleFigure> convert(const std::vector<TriangleFigure> &figs, const Matrix4D &mat) {
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
		Matrix4D eye;
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
			Matrix4D eye;
			ZBuffer zbuf;
			double d, dx, dy;
		} cached;
	};

	struct Lights {
		std::vector<DirectionalLight> directional;
		std::vector<PointLight> point;
		std::vector<ZBufferTriangleFigure> zfigures;
		Matrix4D eye, inv_eye;
		unsigned int shadow_mask;
		Color ambient;
		bool shadows;
	};

	Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection);

	Color color_from_conf(const ini::Section &conf);

	/**
	 * \brief Generic face normal calculator.
	 */
	std::vector<Vector3D> calculate_face_normals(const std::vector<Point3D> &points, const std::vector<Face> &faces);

	void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, const Point3D *points, unsigned int points_len, const Edge *edges, unsigned int edges_len);
	
	template<size_t points_size, size_t edges_size>
	inline void platonic(
		const FigureConfiguration &conf,
		std::vector<Line3D> &lines,
		const std::array<Point3D, points_size> &points,
		const std::array<Edge, edges_size> &edges
	) {
		platonic(conf, lines, points.data(), points_size, edges.data(), edges_size);
	}

	inline void platonic(const FigureConfiguration &conf, std::vector<Line3D> &lines, std::vector<Point3D> points, std::vector<Edge> edges) {
		platonic(conf, lines, points.data(), points.size(), edges.data(), edges.size());
	}
	
	template<size_t points_size, size_t faces_size>
	inline TriangleFigure platonic(
		const FigureConfiguration &conf,
		const std::array<Point3D, points_size> &points,
		const std::array<Face, faces_size> &faces
	) {
		return platonic(conf, { points.begin(), points.end() }, { faces.begin(), faces.end() });
	}

	TriangleFigure platonic(const FigureConfiguration &conf, std::vector<Point3D> points, std::vector<Face> faces);

	img::EasyImage wireframe(const ini::Configuration &, bool with_z);

	img::EasyImage triangles(const ini::Configuration &, bool with_lighting);

	img::EasyImage draw(std::vector<TriangleFigure> figures, Lights lights, unsigned int size, img::Color background);

	std::ostream &operator <<(std::ostream &o, const Color &c);
}
