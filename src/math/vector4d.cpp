#include "math/vector4d.h"
#include <ostream>

using namespace std;

ostream &operator <<(ostream &o, const Vector4D &m) {
	return o << '[' << m.x << ", " << m.y << ", " << m.z << ", " << m.w << ']';
}
