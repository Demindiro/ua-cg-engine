#pragma once

#include <cmath>
#include <vector>
#include "easy_image.h"

class TypeException : public std::exception {
	std::string type;
	
public:
	TypeException(const std::string type) throw();

	virtual const char *what() const throw();
};

// Returns -1 if lower than 0, otherwise 1.
static inline int signum_or_one(int x) {
	return x < 0 ? -1 : 1;
}

static inline img::Color tup_to_color(std::vector<double> v) {
	return img::Color(std::round(v.at(0) * 255), std::round(v.at(1) * 255), std::round(v.at(2) * 255));
}
