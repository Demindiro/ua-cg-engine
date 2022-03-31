#include "math/matrix4d.h"
#include <ostream>

using namespace std;

ostream &operator <<(ostream &o, const Matrix4D &m) {
	o << '[' << m.x() << endl;
	o << ' ' << m.y() << endl;
	o << ' ' << m.z() << endl;
	o << ' ' << m.w() << ']';
	return o;
}
