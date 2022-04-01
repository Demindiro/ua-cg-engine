#include "math/vector2d.h"
#include <ostream>

using namespace std;

ostream &operator <<(ostream &o, const Vector2D &m) {
	return o << '[' << m.x << ", " << m.y << ']';
}
