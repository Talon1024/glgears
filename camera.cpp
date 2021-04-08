#include "camera.h"
#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <GLFW/glfw3.h>
// #include <cstdio>

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
    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);
    glm::quat rotation(glm::vec3(radPhi, 0, -radTheta));
    glm::vec3 movement = rotation * by;
    // std::printf("by: %.3f %.3f %.3f, mv: %.3f %.3f %.3f, theta: %.3f, phi: %.3f\n", by.x, by.y, by.z, movement.x, movement.y, movement.z, theta, phi);
    _position += movement;
}

glm::mat4 CameraHead::getViewMatrix()
{
    glm::vec3 direction = fromSpherical(theta, glm::clamp<float>(phi, -89., 89.));
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
