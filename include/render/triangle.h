#pragma once

#include "render/color.h"
#include "render/rect.h"
#include "render/texture.h"

namespace engine {
namespace render {

struct Face {
	unsigned int a, b, c;
};

class TriangleFigureFlags {
	int flags = 0;

	bool test_flag(unsigned int flag) const {
		return (flags & (1 << flag)) > 0;
	}

	void set_flag(unsigned int flag, bool v) {
		flags &= ~(1 << flag);
		flags |= (unsigned int)v << flag;
	}

public:
	bool separate_normals() const { return test_flag(0); };
	void separate_normals(bool v) { return set_flag(0, v); };
	bool separate_uv() const { return test_flag(1); };
	void separate_uv(bool v) { return set_flag(1, v); };
	bool can_cull() const { return test_flag(2); };
	void can_cull(bool v) { return set_flag(2, v); };
	bool clipped() const { return test_flag(3); };
	void clipped(bool v) { return set_flag(3, v); };
};

struct TriangleFigure {
	std::vector<Point3D> points;
	std::vector<Vector3D> normals;
	std::vector<Point2D> uv;

	std::vector<Face> faces;
	std::vector<Face> faces_normals;
	std::vector<Face> faces_uv;

	std::optional<Texture> texture;
	Color ambient;
	Color diffuse;
	Color specular;
	double reflection;
	unsigned int reflection_int; // Not 0 if defined

	TriangleFigureFlags flags;

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
		: points(fig.points), faces(fig.faces), can_cull(fig.flags.can_cull())
	{}

	ZBufferTriangleFigure(const TriangleFigure &fig, const Matrix4D &mat)
		: faces(fig.faces), can_cull(fig.flags.can_cull())
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
