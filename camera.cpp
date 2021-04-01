#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
// #include <iostream>

static glm::vec3 fromSpherical(float theta, float phi, bool degrees = true)
{
	if (degrees)
	{
		theta = glm::radians(theta);
		phi = glm::radians(phi);
	}
	return glm::vec3(
		glm::sin(theta) * glm::cos(phi),
		glm::cos(theta) * glm::cos(phi),
		glm::sin(phi)
	);
}

void CameraHead::move(glm::vec3 by)
{
	/*
	yaw =
	[ cos x, 0, sin x]
	[ 0,     1, 0    ]
	[-sin x, 0, cos x]

	pitch =
	[ 1,     0,      0]
	[ 0, cos y, -sin y]
	[ 0, sin y,  cos y]

	matrix entry = dot product of row X and column Y
	rotation =
	[cos x    sin x * sin y    sin x * cos y]
	[0        cos y            -sin y       ]
	[-sin x   cos x * sin y    cos x * cos y]
	*/
	/*
	float radPhi = glm::radians(phi);
	float radTheta = glm::radians(theta);
	glm::mat3 rotation(
		cos(radTheta), 0, -sin(radTheta),
		sin(radTheta) * sin(radPhi), cos(radPhi), cos(radTheta) * sin(radPhi),
		sin(radTheta) * cos(radPhi), -sin(radPhi), cos(radTheta) * cos(radPhi));
	glm::mat3 yaw(
		cos(radTheta), 0, -sin(radTheta),
		0, 1, 0,
		sin(radTheta), 0, cos(radTheta));
	glm::mat3 pitch(
		1, 0, 0,
		0, cos(radPhi), sin(radPhi),
		0, -sin(radPhi), cos(radPhi));
	*/
	float radTheta = glm::radians(theta);
	glm::mat2 yaw(
		glm::cos(radTheta), glm::sin(radTheta),
		-glm::sin(radTheta), glm::cos(radTheta)
	);
	glm::vec2 byxy(by.x, by.y);
	_position += glm::vec3(byxy * yaw, 0);
}

glm::mat4 CameraHead::getViewMatrix()
{
	// glm::vec3 offset = -_position;
	glm::vec3 direction = fromSpherical(theta, phi);
	return glm::lookAt(_position, _position + direction, glm::vec3(0, 0, 1));
}

void CameraEye::onWindowResize(GLFWwindow *window, int width, int height)
{
	aspectRatio = (float)width / height;
	fovy = fov / aspectRatio;
}

glm::mat4 CameraEye::getProjectionMatrix(float near, float far)
{
	if (orthographic)
	{
		//return glm::ortho();
	}
	return glm::perspective(glm::radians(fovy), aspectRatio, near, far);
}
