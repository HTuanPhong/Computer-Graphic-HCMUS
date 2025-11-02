#ifndef CAMERA_H
#define CAMERA_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>

struct Camera
{
  glm::vec3 position;
  glm::vec3 worldUp;
  glm::vec3 orientation;
  int width;
  int height;
  float speed;
  float sensitivity;
  float fov;
  bool firstClick;
};

Camera CreateCamera(glm::vec3 position, glm::vec3 worldUp, glm::vec3 orientation, int width, int height);
void SendCameraMatrix(Camera *camera, GLuint ID, const char* uniform);
void ProcessCameraInputs(Camera *camera, GLFWwindow* window);
void ProcessCameraZoom(Camera *camera, float yoffset);

#endif