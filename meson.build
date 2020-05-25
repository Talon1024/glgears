# Meson file for gl_gears2, a OpenGL experiment

project('gl_gears2', 'c', 'cpp', default_options: ['cpp-std=c++11'])

opengl = dependency('GL')
glfw = dependency('glfw3')

glm_path = include_directories('glm')

glad_targ = custom_target('glad',
input: 'dlglad.sh',
output: ['glad.c','glad.h', 'khrplatform.h'],
command: ['zsh', '@INPUT@'],
install: false)

glad_dep = declare_dependency(sources: [glad_targ], link_args: ['-ldl'])

deplist = [opengl, glfw, glad_dep]

executable('gears', 'main.cpp', 'gear.cpp', include_directories: glm_path, dependencies: deplist)