#include "cgengine.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>
#include <vector>
#include "math/point3d.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "shapes.h"
#include "shapes/cylinder.h"
#include "shapes/sphere.h"
#include "shapes/wavefront.h"
#include "render/fragment.h"
#include "render/geometry.h"
#include "render/light.h"
#include "render/triangle.h"
#include "zbuffer.h"

using namespace std;
using namespace engine;
using namespace engine::shapes;
using namespace engine::render;

static Matrix4D isometry_to_matrix(const struct cgengine_isometry3d *iso) {
	auto x = iso->rotation.x, y = iso->rotation.y, z = iso->rotation.z, w = iso->rotation.w;

	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/jay.htm
	// More elegant than calculating each element manually IMO
	// Keep in mind we use *column*-major matrices, so the signs are mirrored diagonally.
	Matrix4D q1 {
		{  w,  z, -y, -x },
		{ -z,  w,  x, -y },
		{  y, -x,  w, -z },
		{  x,  y,  z,  w },
	};
	Matrix4D q2 {
		{  w,  z, -y,  x },
		{ -z,  w,  x,  y },
		{  y, -x,  w,  z },
		{ -x, -y, -z,  w },
	};
	Matrix4D tr {
		{ 1, 0, 0, iso->position.x },
		{ 0, 1, 0, iso->position.y },
		{ 0, 0, 1, iso->position.z },
		{ 0, 0, 0, 1 },
	};

	return (q1 * q2) * tr;
}

extern "C" {

struct cgengine_context {
	vector<TriangleFigure> triangle_figures;
	Lights lights;
	Frustum frustum;
};

struct cgengine_framebuffer {
	img::EasyImage img;
	TaggedZBuffer zbuf;
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
	ctx->frustum.clip(ctx->triangle_figures.back());
}

void cgengine_context_draw(
	const struct cgengine_context *ctx,
	struct cgengine_framebuffer *fb,
	unsigned int mode
) {
	Vector2D offset = { fb->img.get_width() / 2.0, fb->img.get_height() / 2.0 };
	draw(
		ctx->triangle_figures,
		ctx->lights,
		// TODO I added the last division by 2 while debugging why clipping didn't work... and
		// apparently it is necessary. I'm not sure why though, so investigate that.
		fb->img.get_width() / tan(ctx->frustum.fov / 2) / 2,
		offset,
		fb->img,
		fb->zbuf
	);
}

struct cgengine_framebuffer *cgengine_create_framebuffer(unsigned int width, unsigned int height) {
	return new cgengine_framebuffer { { width, height }, { width, height } };
}

void cgengine_destroy_framebuffer(struct cgengine_framebuffer *fb) {
	delete fb;
}

struct cgengine_color8 cgengine_framebuffer_get(
	const struct cgengine_framebuffer * fb,
	unsigned int x,
	unsigned int y
) {
	auto c = fb->img(x, fb->img.get_height() - 1 - y); 
	return { c.b, c.g, c.r };
}

void cgengine_framebuffer_set(
	struct cgengine_framebuffer *fb,
	unsigned int x,
	unsigned int y,
	const struct cgengine_color8 *clr
) {
	fb->img(x, fb->img.get_height() - 1 - y) = { clr->r, clr->g, clr->b };
}

void cgengine_framebuffer_clear(
	struct cgengine_framebuffer *fb,
	const struct cgengine_color8 *bg
) {
	fb->img.clear({ bg->r, bg->g, bg->b });
	fb->zbuf.clear();
}

struct cgengine_material *cgengine_create_material(
	const struct cgengine_texture *,
	const struct cgengine_color *ambient,
	const struct cgengine_color *diffuse,
	const struct cgengine_color *specular,
	double reflection
) {
	return new cgengine_material {{
		{},
		{ ambient->r, ambient->g, ambient->b },
		{ diffuse->r, diffuse->g, diffuse->b },
		{ specular->r, specular->g, specular->b },
		reflection,
	}};
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
		bool point_normals; // TODO
		wavefront(path, shape, m, point_normals);
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

struct cgengine_face_shape *cgengine_face_shape_cylinder(unsigned int n, double height, int point_normals) {
	auto f = new cgengine_face_shape;
	cylinder(n, height, f->shape, point_normals != 0);
	return f;
}

struct cgengine_face_shape *cgengine_face_shape_sphere(unsigned int n, int point_normals) {
	auto f = new cgengine_face_shape;
	sphere(n, f->shape, point_normals != 0);
	return f;
}

void cgengine_error_to_string(struct cgengine_error *err, char *out, size_t n) {
	strncpy(out, err->reason.c_str(), n);
}

void cgengine_destroy_error(struct cgengine_error *err) {
	delete err;
}

}
