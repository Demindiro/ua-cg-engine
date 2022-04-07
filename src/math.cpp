#include <ostream>
#include "math/matrix4d.h"
#include "math/point2d.h"
#include "math/point3d.h"
#include "math/vector2d.h"
#include "math/vector3d.h"
#include "math/vector4d.h"

using namespace std;

ostream &operator <<(ostream &o, const Point2D &m) {
	return o << '(' << m.x << ", " << m.y << ')';
}

ostream &operator <<(ostream &o, const Point3D &m) {
	return o << '(' << m.x << ", " << m.y << ", " << m.z << ')';
}

ostream &operator <<(ostream &o, const Vector2D &m) {
	return o << '[' << m.x << ", " << m.y << ']';
}

ostream &operator <<(ostream &o, const Vector3D &m) {
	return o << '[' << m.x << ", " << m.y << ", " << m.z << ']';
}

ostream &operator <<(ostream &o, const Vector4D &m) {
	return o << '[' << m.x << ", " << m.y << ", " << m.z << ", " << m.w << ']';
}

ostream &operator <<(ostream &o, const Matrix4D &m) {
	o << '[' << m.x() << endl;
	o << ' ' << m.y() << endl;
	o << ' ' << m.z() << endl;
	o << ' ' << m.w() << ']';
	return o;
}
