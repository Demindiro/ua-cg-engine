#include "render/fragment.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/matrix2d.h"
#include "math/matrix4d.h"
#include "math/vector3d.h"
#include "engine.h"
#include "lines.h"
#include "easy_image.h"
#include "render/rect.h"

using namespace std;

namespace engine {
namespace render {

Matrix4D look_direction(Point3D pos, Vector3D dir, Matrix4D &inv) {
	auto r = dir.length();
	auto theta = atan2(-dir.y, -dir.x);
	auto phi = acos(-dir.z / r);

	Matrix4D mat_tr;

	mat_tr(4, 1) = -pos.x;
	mat_tr(4, 2) = -pos.y;
	mat_tr(4, 3) = -pos.z;
	auto mat_rot = Rotation(-(theta + M_PI / 2)).z() * Rotation(-phi).x();

	// A regular view matrix is composed as T * Rz * Rx, hence the inverse
	// matrix is Rx^-1 * Rz^-1 * T^-1.
	//
	// A rotation matrix can simply be transposed to get the inverse.
	//
	// The inverse translation is simply the negated original translation.
	//
	// Calculating this is *much* faster than inverting an arbitrary matrix. It should
	// be more precise too.
	Matrix4D inv_mat_tr;
	inv_mat_tr(4, 1) = pos.x;
	inv_mat_tr(4, 2) = pos.y;
	inv_mat_tr(4, 3) = pos.z;
	inv = mat_rot.transpose() * inv_mat_tr;

	return mat_tr * mat_rot;
}

Matrix4D look_direction(Point3D pos, Vector3D dir) {
	Matrix4D stub;
	return look_direction(pos, dir, stub);
}

}
}
