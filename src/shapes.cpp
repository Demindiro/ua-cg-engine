#include "shapes.h"
#include <algorithm>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <vector>
#include "engine.h"
#include "ini_configuration.h"
#include "math/matrix2d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "render/fragment.h"
#include "render/geometry.h"
#include "render/light.h"
#include "shapes/buckyball.h"
#include "shapes/circle.h"
#include "shapes/cone.h"
#include "shapes/cube.h"
#include "shapes/cylinder.h"
#include "shapes/dodecahedron.h"
#include "shapes/icosahedron.h"
#include "shapes/tetrahedron.h"
#include "shapes/octahedron.h"
#include "shapes/sphere.h"
#include "shapes/torus.h"
#include "wireframe.h"

namespace engine {
namespace shapes {

using namespace std;
using namespace render;

static Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection, Matrix4D &mat_scale) {
	auto rot_x = conf["rotateX"].as_double_or_die() * M_PI / 180;
	auto rot_y = conf["rotateY"].as_double_or_die() * M_PI / 180;
	auto rot_z = conf["rotateZ"].as_double_or_die() * M_PI / 180;
	auto center = tup_to_point3d(conf["center"].as_double_tuple_or_die());
	auto scale = conf["scale"].as_double_or_die();

	// Create transformation matrix
	// Order of operations: scale > rot_x > rot_y > rot_z > translate > project
	// NB the default constructor creates identity matrices (diagonal is 1)
	Matrix4D mat_rot_x, mat_rot_y, mat_rot_z, mat_translate;

	mat_scale(1, 1) = mat_scale(2, 2) = mat_scale(3, 3) = scale;

	mat_rot_x = Rotation(rot_x).x();
	mat_rot_y = Rotation(rot_y).y();
	mat_rot_z = Rotation(rot_z).z();

	mat_translate(4, 1) = center.x;
	mat_translate(4, 2) = center.y;
	mat_translate(4, 3) = center.z;

	return mat_rot_x * mat_rot_y * mat_rot_z * mat_translate * projection;
}

Matrix4D transform_from_conf(const ini::Section &conf, const Matrix4D &projection) {
	Matrix4D mat_scale, mat;
	mat = transform_from_conf(conf, projection, mat_scale);
	return mat_scale * mat;
}

static Color color_from_conf(const vector<double> &c) {
	double b = c.at(2);
	double g = c.at(1);
	double r = c.at(0);
	return { r, g, b };
}

static Color try_color_from_conf(const vector<double> &c) {
	if (c.size() < 3)
		return { 0, 0, 0 };
	double b = c.at(2);
	double g = c.at(1);
	double r = c.at(0);
	return { r, g, b };
}

static Color color_from_conf(const ini::Entry &e) {
	return color_from_conf(e.as_double_tuple_or_die());
}

static Color try_color_from_conf(const ini::Entry &e) {
	vector<double> c;
	if (e.as_double_tuple_if_exists(c)) {
		return color_from_conf(c);
	}
	return Color();
}

Color color_from_conf(const ini::Section &conf) {
	return color_from_conf(conf["color"]);
}

vector<Vector3D> calculate_face_normals(const vector<Point3D> &points, const vector<Face> &faces) {
	vector<Vector3D> normals;
	normals.reserve(faces.size());
	for (auto &f : faces) {
		assert(f.a < points.size());
		assert(f.b < points.size());
		assert(f.c < points.size());
		auto a = points[f.a];
		auto b = points[f.b];
		auto c = points[f.c];
		normals.push_back((b - a).cross(c - a).normalize());
	}
	return normals;
}

void platonic(const FigureConfiguration &conf, vector<Line3D> &lines, const Point3D *points, unsigned int points_len, const Edge *edges, unsigned int edges_len) {
	auto mat = transform_from_conf(conf.section, conf.eye);
	auto color = color_from_conf(conf.section);

	lines.reserve(lines.size() + edges_len);
	for (auto c = edges; c != edges + edges_len; c++) {
		assert(c->a < points_len);
		assert(c->b < points_len);
		// TODO avoid redundant transforms
		auto a = points[c->a] * mat;
		auto b = points[c->b] * mat;
		lines.push_back({{a.x, a.y, a.z}, {b.x, b.y, b.z}, color.to_img_color()});
	}
}

TriangleFigure platonic(const FigureConfiguration &conf, vector<Point3D> points, vector<Face> faces) {

	TriangleFigure fig;
	fig.points = points;
	fig.faces = faces;
	assert(conf.face_normals && "TODO: vertex normals");
	fig.face_normals = conf.face_normals;
	if (conf.with_lighting) {
		fig.ambient = try_color_from_conf(conf.section["ambientReflection"]);
		fig.diffuse = try_color_from_conf(conf.section["diffuseReflection"]);
		fig.specular = try_color_from_conf(conf.section["specularReflection"]);
		fig.reflection = conf.section["reflectionCoefficient"].as_double_or_default(0);
		// integer powers are faster, so try to use that.
		fig.reflection_int = fig.reflection;
		if (fig.reflection_int != fig.reflection) {
			fig.reflection_int = 0;
		}
	} else {
		fig.ambient = color_from_conf(conf.section);
	}
	fig.can_cull = true; // All platonics are solid (& other generated meshes are too)
	fig.clipped = false;

	// Load texture, if any
	string tex_path;
	if (conf.section["texture"].as_string_if_exists(tex_path)) {
		ifstream f(tex_path);
		Texture tex;
		f >> tex.image;
		fig.texture = tex;

		// Generate UVs (flat mapping by default)
		Point2D uv_min, uv_max;
		uv_min.x = uv_min.y = +numeric_limits<double>::infinity();
		uv_max.x = uv_max.y = -numeric_limits<double>::infinity();
		fig.uv.reserve(fig.points.size());
		for (auto &p : fig.points) {
			Point2D uv(p.x, p.z);
			fig.uv.push_back(uv);
			uv_min.x = min(uv_min.x, uv.x);
			uv_min.y = min(uv_min.y, uv.y);
			uv_max.x = max(uv_max.x, uv.x);
			uv_max.y = max(uv_max.y, uv.y);
		}
		for (auto &uv : fig.uv) {
			uv.x = (uv.x - uv_min.x) / (uv_max.x - uv_min.x);
			uv.y = (uv.y - uv_min.y) / (uv_max.y - uv_min.y);
		}
	}

	auto mat = transform_from_conf(conf.section, conf.eye);

	for (auto &p : fig.points) {
		p *= mat;
	}

	fig.normals = calculate_face_normals(fig.points, fig.faces);

	return fig;
}

static void common_conf(
	const ini::Configuration &conf,
	Color &background,
	int &size,
	Matrix4D &mat_eye,
	Matrix4D &mat_inv_eye,
	int &nr_fig,
	Frustum &frustum,
	bool &frustum_use
) {
	background = tup_to_color(conf["General"]["backgroundcolor"].as_double_tuple_or_die());
	size = conf["General"]["size"].as_int_or_die();
	auto eye = tup_to_point3d(conf["General"]["eye"].as_double_tuple_or_die());
	nr_fig = conf["General"]["nrFigures"].as_int_or_die();

	auto clipping = conf["General"]["clipping"].as_bool_or_default(false);

	// Create projection matrix
	Vector3D dir;
	if (clipping) {
		dir = tup_to_vector3d(conf["General"]["viewDirection"].as_double_tuple_or_die());
		frustum_use = true;
		frustum.near = conf["General"]["dNear"].as_double_or_die();
		frustum.far = conf["General"]["dFar"].as_double_or_die();
		frustum.fov = deg2rad(conf["General"]["hfov"].as_double_or_die());
		frustum.aspect = conf["General"]["aspectRatio"].as_double_or_die();
	} else {
		frustum_use = false;
		dir = Point3D() - eye;
	}

	mat_eye = look_direction(eye, dir, mat_inv_eye);
}

img::EasyImage wireframe(const ini::Configuration &conf, bool with_z) {
	Color bg;
	int size, nr_fig;
	Matrix4D mat_eye, mat_inv_eye;
	Frustum frustum;
	bool frustum_use;
	common_conf(conf, bg, size, mat_eye, mat_inv_eye, nr_fig, frustum, frustum_use);

	// Parse figures
	vector<Line3D> lines;
	for (int i = 0; i < nr_fig; i++) {
		auto section = conf[string("Figure") + to_string(i)];
		FigureConfiguration fconf {
			section,
			mat_eye,
			false,
			false,
		};
		auto type = fconf.section["type"].as_string_or_die();
		if (type == "LineDrawing") {
			wireframe::line_drawing(fconf.section, fconf.eye, lines);
		} else if (type == "Cube") {
			cube(fconf, lines);
		} else if (type == "Tetrahedron") {
			tetrahedron(fconf, lines);
		} else if (type == "Octahedron") {
			octahedron(fconf, lines);
		} else if (type == "Icosahedron") {
			icosahedron(fconf, lines);
		} else if (type == "Dodecahedron") {
			dodecahedron(fconf, lines);
		} else if (type == "Cylinder") {
			cylinder(fconf, lines);
		} else if (type == "Cone") {
			cone(fconf, lines);
		} else if (type == "Sphere") {
			sphere(fconf, lines);
		} else if (type == "Torus") {
			torus(fconf, lines);
		} else if (type == "BuckyBall") {
			buckyball(fconf, lines);
		} else if (type == "3DLSystem") {
			wireframe::l_system(fconf.section, fconf.eye, lines);
		} else if (type == "FractalCube") {
			fractal_cube(fconf, lines);
		} else if (type == "FractalOctahedron") {
			fractal_octahedron(fconf, lines);
		} else if (type == "FractalTetrahedron") {
			fractal_tetrahedron(fconf, lines);
		} else if (type == "FractalIcosahedron") {
			fractal_icosahedron(fconf, lines);
		} else if (type == "FractalDodecahedron") {
			fractal_dodecahedron(fconf, lines);
		} else if (type == "FractalBuckyBall") {
			fractal_buckyball(fconf, lines);
		} else if (type == "MengerSponge") {
			puts("TODO MengerSponge lines");
		} else {
			throw TypeException(type);
		}
	}

	// Draw
	return Lines3D(lines).draw(size, bg, with_z);
}

img::EasyImage triangles(const ini::Configuration &conf, bool with_lighting) {
	Color bg;
	int size, nr_fig;
	Matrix4D mat_eye;
	Frustum frustum;
	bool frustum_use;
	Lights lights;

	common_conf(conf, bg, size, mat_eye, lights.inv_eye, nr_fig, frustum, frustum_use);

	// Parse lights
	if (with_lighting) {
		lights.eye = mat_eye;
		lights.shadows = conf["General"]["shadowEnabled"].as_bool_or_default(false);
		lights.shadow_mask = lights.shadows ? conf["General"]["shadowMask"].as_int_or_die(): 0;

		int nr_light = conf["General"]["nrLights"];
		for (int i = 0; i < nr_light; i++) {
			auto section = conf[string("Light") + to_string(i)];
			lights.ambient += color_from_conf(section["ambientLight"]);
			vector<double> diffuse, specular;
			if (section["diffuseLight"].as_double_tuple_if_exists(diffuse)
				| section["specularLight"].as_double_tuple_if_exists(specular)) {
				if (section["infinity"].as_bool_or_default(false)) {
					lights.directional.push_back({
						tup_to_vector3d(section["direction"].as_double_tuple_or_die()).normalize() * mat_eye,
						try_color_from_conf(diffuse),
						try_color_from_conf(specular),
					});
				} else {
					auto d = tup_to_point3d(section["location"].as_double_tuple_or_die());
					auto a = deg2rad(section["spotAngle"].as_double_or_default(90)); // 91 to ensure >= 1.0 works
					d *= mat_eye;
					lights.point.push_back({
						d,
						try_color_from_conf(diffuse),
						try_color_from_conf(specular),
						cos(a),
						{ Matrix4D(), ZBuffer(0, 0), NAN, Vector2D() },
					});
				}
			}
		}
	} else {
		lights.ambient = { 1, 1, 1 };
		lights.shadows = false;
	}
#if GRAPHICS_DEBUG_LIGHT > 0
	{
		lights.ambient = { 1, 1, 1 };
		lights.shadows = false;
		for (auto &p : lights.point) {
			auto pt = p.point * Matrix::inv(mat_eye);
			mat_eye = look_direction(pt, -(pt - Point3D()));
			break;
		}
		if (lights.shadows) {
			size = lights.shadow_mask;
		}
		size = lights.shadow_mask;
		lights.directional.clear();
		lights.point.clear();
		lights.shadows = false;
	}
#endif

	// Parse figures
	vector<TriangleFigure> figures;
	figures.reserve(nr_fig);
	for (int i = 0; i < nr_fig; i++) {
		auto section = conf[string("Figure") + to_string(i)];
		FigureConfiguration fconf {
			section,
			mat_eye,
			with_lighting,
			true,
		};
		auto type = fconf.section["type"].as_string_or_die();
		TriangleFigure fig;
		if (type == "Cube") {
			fig = cube(fconf);
		} else if (type == "Tetrahedron") {
			fig = tetrahedron(fconf);
		} else if (type == "Octahedron") {
			fig = octahedron(fconf);
		} else if (type == "Icosahedron") {
			fig = icosahedron(fconf);
		} else if (type == "Dodecahedron") {
			fig = dodecahedron(fconf);
		} else if (type == "Cylinder") {
			fig = cylinder(fconf);
		} else if (type == "Cone") {
			fig = cone(fconf);
		} else if (type == "Sphere") {
			fig = sphere(fconf);
		} else if (type == "Torus") {
			fig = torus(fconf);
		} else if (type == "BuckyBall") {
			puts("TODO BuckyBall triangles");
			continue;
		} else if (type == "FractalCube") {
			fig = fractal_cube(fconf);
		} else if (type == "FractalTetrahedron") {
			fig = fractal_tetrahedron(fconf);
		} else if (type == "FractalOctahedron") {
			fig = fractal_octahedron(fconf);
		} else if (type == "FractalIcosahedron") {
			fig = fractal_icosahedron(fconf);
		} else if (type == "FractalDodecahedron") {
			fig = fractal_dodecahedron(fconf);
		} else if (type == "FractalBuckyBall") {
			puts("TODO FractalBuckyBall triangles");
			continue;
		} else if (type == "MengerSponge") {
			puts("TODO MengerSponge triangles");
			continue;
		} else {
			throw TypeException(type);
		}
		figures.push_back(fig);
	}

	if (lights.shadows) {
		// We need the full objects for shadowing
		lights.zfigures = ZBufferTriangleFigure::convert(figures);
	}

	// Clipping
	if (frustum_use) {
		for (auto &f : figures) {
			frustum.clip(f);
		}
	}

	// Draw
	return draw(figures, lights, size, bg);
}

}
}
