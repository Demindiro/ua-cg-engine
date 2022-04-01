#pragma once

#include "render/color.h"
#include "render/rect.h"
#include "render/texture.h"

namespace engine {
namespace render {

struct Face {
	unsigned int a, b, c;
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
	/**
	 * \brief Whether each normal is part of a face or a point.
	 */
	bool face_normals;
	bool can_cull;
	bool clipped;

	Rect bounds_projected() const;
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

	Rect bounds_projected() const;

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

}
}
