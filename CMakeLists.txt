cmake_minimum_required(VERSION 3.5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(engine)

############################################################
# Set compiler flags
############################################################
# If supported by your compiler, you can add the -Wall, -Wextra, –fstack-protector-all and -g3 flags here.
set(OWN_GXX_FLAGS "-std=c++17 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${OWN_GXX_FLAGS}")

# Debugging options
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize-undefined-trap-on-error")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG=2")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_FACES=1")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_NORMALS=1")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_Z=1")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DGRAPHICS_DEBUG_LIGHT=1")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${OWN_GXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")

# Generally safe optimizations
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-trapping-math")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-math-errno")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -freciprocal-math") # Technically unsafe but it's fine

# Enable any you're feeling brave :)
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-signed-zeros")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funsafe-math-optimizations")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fassociative-math")

# Or enable this if you are feeling *extra* brave
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${OWN_GXX_FLAGS}")

############################################################
# List all sources
############################################################
include_directories(include)
set(engine_sources
	src/easy_image.cpp
	src/ini_configuration.cpp
	src/intro.cpp
	src/lines.cpp
	src/l_parser.cpp
	src/l_system.cpp
	src/math.cpp
	src/render/color.cpp
	src/render/fragment.cpp
	src/render/fragment/edges.cpp
	src/render/fragment/faces.cpp
	src/render/geometry.cpp
	src/render/rect.cpp
	src/render/triangle.cpp
	src/shapes.cpp
	src/shapes/cone.cpp
	src/shapes/circle.cpp
	src/shapes/cylinder.cpp
	src/shapes/fractal.cpp
	src/shapes/mengersponge.cpp
	src/shapes/sphere.cpp
	src/shapes/thicken.cpp
	src/shapes/torus.cpp
	src/shapes/wavefront.cpp
	src/wireframe.cpp
	src/zbuffer.cpp
)

set(engine_standalone_sources
	${engine_sources}
	src/engine.cpp
)

set(engine_library_sources
	${engine_sources}
	src/cgengine.cpp
)

set(exe_name "engine")
set(lib_name "cgengine")
add_executable( ${exe_name} ${engine_standalone_sources} )
add_library( ${lib_name} ${engine_library_sources} )
#install( TARGETS ${exe_name} DESTINATION ${PROJECT_SOURCE_DIR}/ )
