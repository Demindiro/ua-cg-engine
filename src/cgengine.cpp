#include "cgengine.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <vector>
#include "math/point3d.h"
#include "math/vector3d.h"
#include "shapes.h"
#include "shapes/wavefront.h"
#include "render/fragment.h"
#include "render/geometry.h"
#include "render/light.h"
#include "render/triangle.h"

using namespace std;
using namespace engine::shapes;
using namespace engine::render;

static Matrix4D isometry_to_matrix(const struct cgengine_isometry3d *iso) {
	auto x = iso->rotation.x, y = iso->rotation.y, z = iso->rotation.z, w = iso->rotation.w;

	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/jay.htm
	// More elegant than calculating each element manually IMO
	// Keep in mind we use *column*-major matrices, so the signs are mirrored diagonally.
	Matrix4D q1 {
		{  w,  z, -y, -x },
		{ -z,  w,  y, -y },
		{  y, -x,  w, -z },
		{  x,  y,  z,  w },
	};
	Matrix4D q2 {
		{  w,  z, -y,  x },
		{ -z,  w,  y,  y },
		{  y, -x,  w,  z },
		{ -x, -y, -z,  w },
	};
	Matrix4D tr {
		{ 1, 0, 0, iso->position.x },
		{ 0, 1, 0, iso->position.y },
		{ 0, 0, 1, iso->position.z },
		{ 0, 0, 0, 1 },
	};

	return tr * (q1 * q2);
}

extern "C" {

struct cgengine_context {
	vector<TriangleFigure> triangle_figures;
	Lights lights;
	Frustum frustum;
};

struct cgengine_error {
	string reason;
};

struct cgengine_face_shape {
	FaceShape shape;
};

struct cgengine_material {
	Material mat;
};

struct cgengine_framebuffer {
	unsigned int width, height;
	cgengine_color8 *buffer;
};

struct cgengine_context *cgengine_create_context() {
	auto ctx = new cgengine_context;
	ctx->lights.ambient = { 1, 1, 1 };
	return ctx;
}

void cgengine_destroy_context(struct cgengine_context *ctx) {
	delete ctx;
}

void cgengine_context_clear_figures(struct cgengine_context *ctx) {
	ctx->triangle_figures.clear();
}

void cgengine_context_set_camera(struct cgengine_context *ctx, double fov, double aspect, double near, double far) {
	ctx->frustum.fov = fov;
	ctx->frustum.aspect = aspect;
	ctx->frustum.near = near;
	ctx->frustum.far = far;
}

void cgengine_context_move_camera(
	struct cgengine_context *ctx,
	const struct cgengine_point3d *pos,
	const struct cgengine_vector3d *dir
) {
	Point3D p(pos->x, pos->y, pos->z);
	Vector3D d(dir->x, dir->y, dir->z);
	ctx->lights.eye = look_direction(p, d, ctx->lights.inv_eye);
}

void cgengine_context_add_face_shape(
	struct cgengine_context *ctx,
	const struct cgengine_face_shape *shape,
	const struct cgengine_material *mat,
	const struct cgengine_isometry3d *iso,
	double scale,
	int flags
) {
	auto m = isometry_to_matrix(iso) * ctx->lights.eye;
	bool with_point_normals = (flags & 1) > 0;
	bool with_cubemap = (flags & 2) > 0;
	ctx->triangle_figures.push_back(convert(shape->shape, mat->mat, m, scale, with_cubemap, with_point_normals));
}

void cgengine_context_draw(
	const struct cgengine_context *ctx,
	struct cgengine_framebuffer *fb,
	unsigned int mode
) {
	auto img = draw(ctx->triangle_figures, ctx->lights, 256, Color());
	for (unsigned int y = 0; y < min(img.get_height(), fb->height); y++) {
		for (unsigned int x = 0; x < min(img.get_width(), fb->width); x++) {
			auto c = img(x, img.get_height() - 1 - y);
			struct cgengine_color8 clr { c.b, c.g, c.r };
			cgengine_framebuffer_set(fb, x, y, &clr);
		}
	}
}

struct cgengine_framebuffer *cgengine_create_framebuffer(size_t width, size_t height) {
	return new cgengine_framebuffer { width, height, new cgengine_color8[width * height] };
}

void cgengine_destroy_framebuffer(struct cgengine_framebuffer *fb) {
	if (fb != nullptr) {
		delete[] fb->buffer;
	}
	delete fb;
}

void cgengine_framebuffer_clear(
	struct cgengine_framebuffer *fb,
	const struct cgengine_color8 *bg
) {
	size_t s = (size_t)fb->height * fb->width;
	for (size_t i = 0; i < s; i++) {
		fb->buffer[i] = *bg;
	}
}

struct cgengine_color8 cgengine_framebuffer_get(
	const struct cgengine_framebuffer *fb,
	unsigned int x,
	unsigned int y
) {
	return fb->buffer[y * fb->width + x];
}

/**
 * \brief Set a pixel in a framebuffer.
 */
void cgengine_framebuffer_set(
	const struct cgengine_framebuffer *fb,
	unsigned int x,
	unsigned int y,
	const struct cgengine_color8 *clr
) {
	fb->buffer[y * fb->width + x] = *clr;
}

void cgengine_destroy_material(struct cgengine_material *mat) {
	delete mat;
}

void cgengine_destroy_face_shape(struct cgengine_face_shape *shape) {
	delete shape;
}

struct cgengine_load_face_shape_result cgengine_face_shape_load(
	const char *path,
	enum cgengine_load_type type,
	struct cgengine_material **mat
) {
	struct cgengine_load_face_shape_result res;
	try {
		FaceShape shape;
		Material m;
		wavefront(path, shape, m);
		res.success = 1;
		res.shape = new cgengine_face_shape { move(shape) };
		if (mat != nullptr) {
			*mat = new cgengine_material { move(m) };
		}
	} catch (exception &e) {
		res.success = 0;
		res.error = new cgengine_error { {e.what()} };
	}
	return res;
}

void cgengine_error_to_string(struct cgengine_error *err, char *out, size_t n) {
	strncpy(out, err->reason.c_str(), n);
}

void cgengine_destroy_error(struct cgengine_error *err) {
	delete err;
}

}
