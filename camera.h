#pragma once
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
// Camera position - view matrix
class CameraHead {
private:
	glm::vec3 _position;
public:
	float theta, phi;
	void move(glm::vec3 by);
	glm::mat4 getViewMatrix();
};

// Camera eye - projection matrix
class CameraEye {
private:
	float aspectRatio;
	float fovy;
public:
	float fov = 100;
	bool orthographic;
	void onWindowResize(GLFWwindow* window, int width, int height);
	glm::mat4 getProjectionMatrix(float near = 0.03125, float far = 10000);
};
