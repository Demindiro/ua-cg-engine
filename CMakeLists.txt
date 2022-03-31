cmake_minimum_required(VERSION 3.5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(engine)

############################################################
# Set compiler flags
############################################################
# If supported by your compiler, you can add the -Wall, -Wextra, –fstack-protector-all and -g3 flags here.
set(OWN_GXX_FLAGS "-std=c++17 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OWN_GXX_FLAGS} -g")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG=2")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_FACES=2")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_NORMALS=1")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_Z=1")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_LIGHT=1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OWN_GXX_FLAGS} -flto -march=native -fno-trapping-math -fno-math-errno -freciprocal-math")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${OWN_GXX_FLAGS}")

############################################################
# List all sources
############################################################
include_directories(include)
set(engine_sources
	src/engine.cpp
	src/easy_image.cpp
	src/ini_configuration.cpp
	src/intro.cpp
	src/lines.cpp
	src/l_parser.cpp
	src/l_system.cpp
	src/math/matrix4d.cpp
	src/math/point2d.cpp
	src/math/vector4d.cpp
	src/shapes.cpp
	src/shapes/buckyball.cpp
	src/shapes/cone.cpp
	src/shapes/circle.cpp
	src/shapes/cube.cpp
	src/shapes/cylinder.cpp
	src/shapes/dodecahedron.cpp
	src/shapes/fractal.cpp
	src/shapes/icosahedron.cpp
	src/shapes/tetrahedron.cpp
	src/shapes/octahedron.cpp
	src/shapes/sphere.cpp
	src/shapes/torus.cpp
	src/wireframe.cpp
	src/zbuffer.cpp
)

set(exe_name "engine")
add_executable( ${exe_name} ${engine_sources} )
#install( TARGETS ${exe_name} DESTINATION ${PROJECT_SOURCE_DIR}/ )
