#pragma once

#include <optional>
#include "math/point3d.h"
#include "math/matrix4d.h"
#include "render/color.h"
#include "render/texture.h"
#include "render/triangle.h"

namespace engine {
namespace render {

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
		double d;
		Vector2D offset;
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
	std::optional<Texture> cubemap;
};

}
}
