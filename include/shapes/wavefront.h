#pragma once

#include <exception>
#include <string>
#include "ini_configuration.h"
#include "shapes.h"

namespace engine {
namespace shapes {

class WavefrontParseException : public std::exception {
	std::string reason;
	friend void wavefront(const Configuration &conf, FaceShape &shape, Material &mat);
	WavefrontParseException(unsigned int line_i, std::string reason)
		: reason(reason + " @ line " + std::to_string(line_i)) {}

public:
	const char *what() const noexcept {
		return reason.data();
	}
};

void wavefront(const Configuration &conf, FaceShape &shape, Material &mat);

}
}
