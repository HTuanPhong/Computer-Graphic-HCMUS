#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Camera
{
  glm::vec3 position;    // computed from spherical coords
  glm::vec3 target;      // usually (0,0,0)
  float distance;        // orbit radius
  float yaw;             // horizontal angle (in degrees)
  float pitch;           // vertical angle (in degrees)
  float sensitivity;     // mouse sensitivity
  float zoomSpeed;       // scroll sensitivity
  int width, height;     // window size
  bool firstClick;
};

Camera CreateOrbitCamera(glm::vec3 target, float distance, int width, int height);
void SendCameraMatrix(Camera* camera, GLuint shaderID, const char* uniform, bool ortho);
void ProcessOrbitCamera(Camera* camera, GLFWwindow* window);
void ProcessOrbitZoom(Camera* camera, float yoffset);

#endif
