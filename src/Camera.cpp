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

static glm::mat4 mixMat4(const glm::mat4& a, const glm::mat4& b, float t)
{
    return a * (1.0f - t) + b * t;
}

void SendCameraMatrix(Camera* camera, GLuint shaderID, const char* uniform, float t)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    glm::mat4 view = glm::lookAt(camera->position, camera->target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection;
    float aspect = (float)camera->width / (float)camera->height;

    // --- Define the two key matrices: Perspective and a CORRECTLY SIZED Orthographic ---

    // 1. Pure Perspective Matrix (for t = 0.0)
    float fovRadians = glm::radians(45.0f);
    glm::mat4 perspectiveMatrix = glm::perspective(fovRadians, aspect, 0.1f, 100.0f);

    // 2. Pure Orthographic Matrix (for t = 0.5)
    //    Calculate the size of the ortho view so that it perfectly matches
    //    the perspective view at the target distance.
    float distanceToTarget = glm::distance(camera->position, camera->target);
    float orthoHeight = distanceToTarget * tan(fovRadians / 2.0f);
    float orthoWidth = orthoHeight * aspect;
    glm::mat4 orthoMatrix = glm::ortho(-orthoWidth, orthoWidth,
                                       -orthoHeight, orthoHeight,
                                       0.1f, 100.0f);

    if (t <= 0.5f) {
        // --- Interpolate between Perspective and the correctly-sized Orthographic ---
        float lerpT = t * 2.0f; 
        projection = mixMat4(perspectiveMatrix, orthoMatrix, lerpT);
    } else {
        // --- Interpolate between Orthographic and Oblique ---
        float lerpT = (t - 0.5f) * 2.0f;

        // The oblique matrix is built from the same correctly-sized ortho matrix
        float angleRad = glm::radians(30.0);
        float sx = -0.5 * cos(angleRad);
        float sy = -0.5 * sin(angleRad);
        glm::mat4 shearMatrix = glm::mat4(1.0f);
        shearMatrix[2][0] = sx;
        shearMatrix[2][1] = sy;
        glm::mat4 counterTranslateMatrix = glm::translate(glm::mat4(1.0f), 
            glm::vec3(sx * distanceToTarget, sy * distanceToTarget, 0.0f));
        glm::mat4 obliqueMatrix = orthoMatrix * shearMatrix * counterTranslateMatrix;
        
        projection = mixMat4(orthoMatrix, obliqueMatrix, lerpT);
    }
    camera->camRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
    camera->camUp = glm::vec3(view[0][1], view[1][1], view[2][1]);
    camera->camFront = glm::vec3(view[0][2], view[1][2], view[2][2]);
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
    if (camera->distance < 0.1f)  camera->distance = 0.1f;
    if (camera->distance > 50.0f) camera->distance = 50.0f;

    // update position after zoom change
    float yawRad = glm::radians(camera->yaw);
    float pitchRad = glm::radians(camera->pitch);
    camera->position.x = camera->target.x + camera->distance * cosf(pitchRad) * sinf(yawRad);
    camera->position.y = camera->target.y + camera->distance * sinf(pitchRad);
    camera->position.z = camera->target.z + camera->distance * cosf(pitchRad) * cosf(yawRad);
}
