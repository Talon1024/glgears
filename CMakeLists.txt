cmake_minimum_required(VERSION 3.16)
include(FindPkgConfig)
project(gl_gears2)

set(SOURCES main.cpp gear.cpp)

set(GLAD_SOURCES glad.c)

add_custom_target(glad
	COMMAND ${CMAKE_SOURCE_DIR}/dlglad.sh
	OUTPUT ${GLAD_SOURCES} glad.h khrplatform.h
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

pkg_check_modules(glfw3 REQUIRED)
cmake_policy(SET CMP0072 NEW)
include(FindOpenGL)

add_executable(gl_gears2 ${SOURCES} ${GLAD_SOURCES})
target_include_directories(gl_gears2 PUBLIC glm ${CMAKE_BINARY_DIR})
add_dependencies(gl_gears2 ${glfw3_LIBRARIES} OpenGL::GL glad)
target_link_libraries(gl_gears2 ${glfw3_LINK_LIBRARIES} dl)
