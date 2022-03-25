#pragma once

#include "easy_image.h"
#include "ini_configuration.h"

namespace wireframe {
	img::EasyImage wireframe(const ini::Configuration &, bool with_z);
}
