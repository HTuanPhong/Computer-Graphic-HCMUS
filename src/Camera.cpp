#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <cmath>

Camera CreateOrbitCamera(glm::vec3 target, float distance, int width, int height)
{
    Camera cam{};
    cam.target = target;
    cam.distance = distance;
    cam.yaw = 0.0f;          // start facing +Z
    cam.pitch = 20.0f;       // slight downward tilt
    cam.sensitivity = 0.1f;
    cam.zoomSpeed = 2.0f;
    cam.width = width;
    cam.height = height;
    cam.firstClick = true;

    // calculate initial position
    float yawRad = glm::radians(cam.yaw);
    float pitchRad = glm::radians(cam.pitch);
    cam.position.x = cam.target.x + cam.distance * cosf(pitchRad) * sinf(yawRad);
    cam.position.y = cam.target.y + cam.distance * sinf(pitchRad);
    cam.position.z = cam.target.z + cam.distance * cosf(pitchRad) * cosf(yawRad);

    return cam;
}

void SendCameraMatrix(Camera* camera, GLuint shaderID, const char* uniform, bool ortho)
{
    glm::mat4 view = glm::lookAt(camera->position, camera->target, glm::vec3(0, 1, 0));
    glm::mat4 projection;

    float aspect = (float)camera->width / (float)camera->height;
    if (ortho) {
        float orthoSize = camera->distance * 0.5f;
        projection = glm::ortho(-orthoSize * aspect, orthoSize * aspect,
                                -orthoSize, orthoSize,
                                0.1f, 100.0f);
    } else {
        projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }

    glm::mat4 camMatrix = projection * view;
    glUniformMatrix4fv(glGetUniformLocation(shaderID, uniform), 1, GL_FALSE, glm::value_ptr(camMatrix));
}

void ProcessOrbitCamera(Camera* camera, GLFWwindow* window)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (camera->firstClick)
        {
            glfwSetCursorPos(window, camera->width / 2.0, camera->height / 2.0);
            camera->firstClick = false;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float deltaX = (float)(mouseX - camera->width / 2.0);
        float deltaY = (float)(mouseY - camera->height / 2.0);

        camera->yaw   -= deltaX * camera->sensitivity;
        camera->pitch += deltaY * camera->sensitivity;

        // limit pitch to avoid flipping
        if (camera->pitch > 89.0f) camera->pitch = 89.0f;
        if (camera->pitch < -89.0f) camera->pitch = -89.0f;

        // recompute position from spherical coords
        float yawRad = glm::radians(camera->yaw);
        float pitchRad = glm::radians(camera->pitch);

        camera->position.x = camera->target.x + camera->distance * cosf(pitchRad) * sinf(yawRad);
        camera->position.y = camera->target.y + camera->distance * sinf(pitchRad);
        camera->position.z = camera->target.z + camera->distance * cosf(pitchRad) * cosf(yawRad);

        // recenter cursor
        glfwSetCursorPos(window, camera->width / 2.0, camera->height / 2.0);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        camera->firstClick = true;
    }
}

void ProcessOrbitZoom(Camera* camera, float yoffset)
{
    camera->distance -= yoffset * camera->zoomSpeed;
    if (camera->distance < 1.0f)  camera->distance = 1.0f;
    if (camera->distance > 50.0f) camera->distance = 50.0f;

    // update position after zoom change
    float yawRad = glm::radians(camera->yaw);
    float pitchRad = glm::radians(camera->pitch);
    camera->position.x = camera->target.x + camera->distance * cosf(pitchRad) * sinf(yawRad);
    camera->position.y = camera->target.y + camera->distance * sinf(pitchRad);
    camera->position.z = camera->target.z + camera->distance * cosf(pitchRad) * cosf(yawRad);
}
