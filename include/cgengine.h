#ifndef CGENGINE_H
#define CGENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * \brief C interface for the engine.
 *
 * C is chosen since it's useable with more than just a specific version of C++
 * and even other languages.
 *
 * All methods are prefixed with `cgengine_`
 */

typedef double cgengine_real_t;

struct cgengine_point3d {
	cgengine_real_t x, y, z;
};

struct cgengine_vector3d {
	cgengine_real_t x, y, z;
};

struct cgengine_rotation3d {
	cgengine_real_t x, y, z, w;
};

struct cgengine_isometry3d {
	struct cgengine_point3d position;
	struct cgengine_rotation3d rotation;
};

struct cgengine_context;
struct cgengine_face_shape;
struct cgengine_material;

enum cgengine_draw_mode {
	CGENGINE_DRAW_LIGHTED_ZBUFFER,
};

enum cgengine_load_type {
	CGENGINE_WAVEFRONT,
};

struct cgengine_error;

struct cgengine_load_face_shape_result {
	char success;
	union {
		struct cgengine_face_shape *shape;
		struct cgengine_error *error;
	};
};

struct cgengine_framebuffer;

struct cgengine_color {
	cgengine_real_t b, g, r;
};

struct cgengine_color8 {
	uint8_t b, g, r;
};

/**
 * \brief Create a new rendering context.
 *
 * A context contains a collection of shapes to be rendered.
 */
struct cgengine_context *cgengine_create_context();

/**
 * \brief Destroy a context created with cgengine_new_context.
 */
void cgengine_destroy_context(struct cgengine_context *);

/**
 * \brief Clear all figures in a context.
 */
void cgengine_context_clear_figures(struct cgengine_context *);

/**
 * \brief Set the field of view, aspect, near plane and far plane of the camera.
 */
void cgengine_context_set_camera(struct cgengine_context *, double fov, double aspect, double near, double far);

/**
 * \brief Set the location and direction of the camera.
 */
void cgengine_context_move_camera(
	struct cgengine_context *,
	const struct cgengine_point3d *pos,
	const struct cgengine_vector3d *dir
);

/**
 * \brief Add a face shape to a context to be rendered.
 */
void cgengine_context_add_face_shape(
	struct cgengine_context *,
	const struct cgengine_face_shape *,
	const struct cgengine_material *,
	const struct cgengine_isometry3d *,
	double scale,
	int flags
);

/**
 * \brief Destroy a face shape.
 */
void cgengine_destroy_face_shape(struct cgengine_face_shape *);

/**
 * \brief Draw a context with a given mode.
 */
void cgengine_context_draw(
	const struct cgengine_context *,
	struct cgengine_framebuffer *,
	unsigned int mode
);

/**
 * \brief Create a framebuffer to draw to.
 */
struct cgengine_framebuffer *cgengine_create_framebuffer(unsigned int width, unsigned int height);

/**
 * \brief Destroy a framebuffer.
 */
void cgengine_destroy_framebuffer(struct cgengine_framebuffer *);

/**
 * \brief Get a pixel in a framebuffer.
 */
struct cgengine_color8 cgengine_framebuffer_get(
	const struct cgengine_framebuffer * fb,
	unsigned int x,
	unsigned int y
);

/**
 * \brief Set a pixel in a framebuffer.
 */
void cgengine_framebuffer_set(
	struct cgengine_framebuffer *fb,
	unsigned int x,
	unsigned int y,
	const struct cgengine_color8 *clr
);

/**
 * \brief Fill a framebuffer with a color.
 */
void cgengine_framebuffer_clear(
	struct cgengine_framebuffer *fb,
	const struct cgengine_color8 *bg
);

/**
 * \brief Try to load a face shape from a file.
 *
 * Supported formats:
 * - Wavefront object file (.obj).
 */
struct cgengine_load_face_shape_result cgengine_face_shape_load(
	const char *,
	enum cgengine_load_type,
	struct cgengine_material **
);

/**
 * \brief Create a cylinder shape.
 *
 * The base of the cylinder is located at the origin.
 *
 * \param n The amount of side faces.
 * \param height The height of the cylinder.
 * \param point_normals Whether to use face or point normals, i.e. flat or smooth shading.
 */
struct cgengine_face_shape *cgengine_face_shape_cylinder(unsigned int n, double height, int point_normals);

/**
 * \brief Create a sphere shape.
 *
 * The sphere is created by bisecting the edges of an icosahedron, then normalizing the distance
 * of the points to the center.
 *
 * \param n How many times to bisect the edges.
 * \param point_normals Whether to use face or point normals, i.e. flat or smooth shading.
 */
struct cgengine_face_shape *cgengine_face_shape_sphere(unsigned int n, int point_normals);

/**
 * \brief Create a material.
 */
struct cgengine_material *cgengine_create_material(
	const struct cgengine_texture *,
	const struct cgengine_color *ambient,
	const struct cgengine_color *diffuse,
	const struct cgengine_color *specular,
	double reflection
);

/**
 * \brief Destroy a material.
 */
void cgengine_destroy_material(struct cgengine_material *mat);

/**
 * \brief Convert an error to a human-readable string.
 */
void cgengine_error_to_string(struct cgengine_error *, char *out, size_t n);

/**
 * \brief Destroy an error object.
 */
void cgengine_destroy_error(struct cgengine_error *);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <exception>
#include <string>

/**
 * \brief Convienence classes for C++
 */
namespace cgengine {

typedef struct cgengine_point3d Point3D;
typedef struct cgengine_vector3d Vector3D;
typedef struct cgengine_rotation3d Rotation3D;
typedef struct cgengine_isometry3d Isometry3D;
typedef struct cgengine_color Color;
typedef struct cgengine_color8 Color8;

static inline Color color(double r, double g, double b) {
	cgengine_color c;
	c.r = r;
	c.b = b;
	c.g = g;
	return c;
}

class Error : public std::exception {
	struct cgengine_error *err;
	mutable char buf[256];
	friend class Context;
	friend class FaceShape;
	friend class Framebuffer;
	Error(struct cgengine_error *err) : err(err) {}

	Error(const Error &) {}

	Error &operator =(const Error &) { return *this; }

public:
	virtual ~Error() throw() {
		cgengine_destroy_error(err);
	}

	virtual const char *what() const throw() {
		cgengine_error_to_string(err, buf, sizeof(buf));
		return buf;
	}
};

class Framebuffer {
	Framebuffer(const Framebuffer &) {}

	Framebuffer &operator =(const Framebuffer &) { return *this; }

	friend class Context;
	struct cgengine_framebuffer *fb;

public:
	Framebuffer() : fb(NULL) {}

	Framebuffer(unsigned int width, unsigned int height) {
		fb = cgengine_create_framebuffer(width, height);
	}

	~Framebuffer() {
		cgengine_destroy_framebuffer(fb);
	}

	void resize(unsigned int width, unsigned int height) {
		cgengine_destroy_framebuffer(fb);
		fb = cgengine_create_framebuffer(width, height);
	}

	Color8 get(unsigned int x, unsigned int y) {
		return cgengine_framebuffer_get(fb, x, y);
	}

	void set(unsigned int x, unsigned int y, const Color8 &clr) {
		cgengine_framebuffer_set(fb, x, y, &clr);
	}

	void clear(const Color8 &clr) {
		cgengine_framebuffer_clear(fb, &clr);
	}
};

class Material {
	Material(const Material &) {}

	Material &operator =(const Material &) { return *this; }

	friend class Context;
	friend class FaceShape;
	struct cgengine_material *mat;

public:
	Material() : mat(NULL) {}

	Material(const Color &ambient, const Color &diffuse, const Color &specular, double reflection)
		: mat(cgengine_create_material(NULL, &ambient, &diffuse, &specular, reflection))
	{}

	~Material() {
		cgengine_destroy_material(mat);
	}

	void move(Material &m) {
		cgengine_destroy_material(mat);
		mat = m.mat;
		m.mat = NULL;
	}
};

class FaceShape {
	FaceShape(struct cgengine_face_shape *shape) : shape(shape) {}

	FaceShape(const FaceShape &) {}

	FaceShape &operator =(const FaceShape &) { return *this; }

	friend class Context;
	struct cgengine_face_shape *shape;

public:
	FaceShape() : shape(NULL) {}
		
	~FaceShape() {
		cgengine_destroy_face_shape(shape);
	}

	FaceShape(const char *path, enum cgengine_load_type type, Material &mat) {
		struct cgengine_load_face_shape_result res = cgengine_face_shape_load(path, type, &mat.mat);
		if (!res.success) {
			throw Error(res.error);
		}
		shape = res.shape;
	}

	void move(FaceShape &f) {
		cgengine_destroy_face_shape(shape);
		shape = f.shape;
		f.shape = NULL;
	}

	void cylinder(unsigned int n, double height, bool point_normals) {
		cgengine_destroy_face_shape(shape);
		shape = cgengine_face_shape_cylinder(n, height, point_normals);
	}

	void sphere(unsigned int n, bool point_normals) {
		cgengine_destroy_face_shape(shape);
		shape = cgengine_face_shape_sphere(n, point_normals);
	}
};

class Context {
	struct cgengine_context *ctx;

	Context(const Context &) {}

	Context &operator =(const Context &) { return *this; }

public:
	Context() : ctx(cgengine_create_context()) {}

	~Context() {
		cgengine_destroy_context(ctx);
	}

	void move(Context &c) {
		cgengine_destroy_context(ctx);
		ctx = c.ctx;
		c.ctx = NULL;
	}

	void move_camera(const Point3D &position, const Vector3D &direction) {
		cgengine_context_move_camera(ctx, &position, &direction);
	}

	void set_camera(double fov, double aspect, double near, double far) {
		cgengine_context_set_camera(ctx, fov, aspect, near, far);
	}

	void add_shape(const FaceShape &shape, const Material &mat, const Isometry3D &iso, double scale, int flags) {
		cgengine_context_add_face_shape(ctx, shape.shape, mat.mat, &iso, scale, flags);
	}

	void draw(Framebuffer &fb, int mode) const {
		cgengine_context_draw(ctx, fb.fb, mode);
	}

	void clear_figures() {
		cgengine_context_clear_figures(ctx);
	}
};

}

#endif

#endif
