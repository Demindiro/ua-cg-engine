#include "math/vector3d.h"
#include <ostream>

using namespace std;

ostream &operator <<(ostream &o, const Vector3D &m) {
	return o << '[' << m.x << ", " << m.y << ", " << m.z << ']';
}
