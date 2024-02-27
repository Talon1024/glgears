#pragma once
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// For view and projection matrices
class Camera {
    private:
    float aspectRatio;
    float fovy;

    public:
    float theta, phi;
    float fov = 100;
    bool orthographic;
    glm::vec3 position;

    void move(glm::vec3 by);
    glm::mat4 getViewMatrix();
    void onWindowResize(GLFWwindow* window, int width, int height);
    glm::mat4 getProjectionMatrix(float near = 0.03125, float far = 10000);
    glm::mat4 getViewProjMatrix(float near = 0.03125, float far = 10000) {
        return getProjectionMatrix(near, far) * getViewMatrix();
    }
};
