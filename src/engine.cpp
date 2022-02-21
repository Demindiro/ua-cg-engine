#include "easy_image.h"
#include "engine.h"
#include "ini_configuration.h"
#include "intro.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

TypeException::TypeException(const std::string type) throw() : std::exception(), type(type) {}

const char *TypeException::what() const throw() {
	return type.c_str();
};

img::EasyImage generate_image(const ini::Configuration &conf) {
	auto width  = conf["ImageProperties"]["width" ].as_int_or_die();
	auto height = conf["ImageProperties"]["height"].as_int_or_die();
	img::EasyImage img(width, height);

	auto type = conf["General"]["type"].as_string_or_die();

	if (type == "IntroColorRectangle") {
		intro::color_rectangle(img, conf);
	} else if (type == "IntroBlocks") {
		intro::blocks(img, conf);
	} else if (type == "IntroLines") {
		intro::lines(img, conf);
	} else {
		throw TypeException(type);
	}

	return img;
}

int main(int argc, char const *argv[]) {
	int retVal = 0;
	try {
		std::vector<std::string> args =
			std::vector<std::string>(argv + 1, argv + argc);
		if (args.empty()) {
			std::ifstream fileIn("filelist");
			std::string filelistName;
			while (std::getline(fileIn, filelistName)) {
				args.push_back(filelistName);
			}
		}
		for (std::string fileName : args) {
			ini::Configuration conf;
			try {
				std::ifstream fin(fileName);
				fin >> conf;
				fin.close();
			} catch (ini::ParseException &ex) {
				std::cerr << "Error parsing file: " << fileName << ": "
						  << ex.what() << std::endl;
				retVal = 1;
				continue;
			}

			img::EasyImage image = generate_image(conf);
			if (image.get_height() > 0 && image.get_width() > 0) {
				std::string::size_type pos = fileName.rfind('.');
				if (pos == std::string::npos) {
					// filename does not contain a '.' --> append a '.bmp'
					// suffix
					fileName += ".bmp";
				} else {
					fileName = fileName.substr(0, pos) + ".bmp";
				}
				try {
					std::ofstream f_out(fileName.c_str(), std::ios::trunc |
															  std::ios::out |
															  std::ios::binary);
					f_out << image;

				} catch (std::exception &ex) {
					std::cerr << "Failed to write image to file: " << ex.what()
							  << std::endl;
					retVal = 1;
				}
			} else {
				std::cout << "Could not generate image for " << fileName
						  << std::endl;
			}
		}
	} catch (const std::bad_alloc &exception) {
		// When you run out of memory this exception is thrown. When this
		// happens the return value of the program MUST be '100'. Basically this
		// return value tells our automated test scripts to run your engine on a
		// pc with more memory. (Unless of course you are already consuming the
		// maximum allowed amount of memory) If your engine does NOT adhere to
		// this requirement you risk losing points because then our scripts will
		// mark the test as failed while in reality it just needed a bit more
		// memory
		std::cerr << "Error: insufficient memory" << std::endl;
		retVal = 100;
	}
	return retVal;
}
