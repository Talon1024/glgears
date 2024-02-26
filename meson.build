# Meson file for gl_gears2, a OpenGL experiment

project('gl_gears2', 'c', 'cpp', default_options: ['cpp_std=c++11'])

opengl = dependency('GL')
glfw = dependency('glfw3')

glm_path = include_directories('glm')
glad_path = include_directories('glad')

glad_dep = declare_dependency(
	sources: [
		'glad/glad.c',
		'glad/glad.h',
		'glad/khrplatform.h'
	],
	link_args: ['-ldl']
)

deplist = [opengl, glfw, glad_dep]

executable('gears',
	'main.cpp', 'gear.cpp', 'input.cpp', 'camera.cpp', '3dobject.cpp',
	include_directories: [glm_path, glad_path], dependencies: deplist)