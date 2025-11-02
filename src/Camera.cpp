#include "Camera.hpp"
#include "App.hpp"
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>
Camera CreateCamera(glm::vec3 position, glm::vec3 worldUp, glm::vec3 orientation, int width, int height)
{
  Camera camera;
  camera.position = position;
  camera.worldUp = worldUp;
  camera.orientation = orientation;
  camera.width = width;
  camera.height = height;
  camera.fov = 45.0f;
	camera.sensitivity = 60.0f;
	camera.speed = 0.1f;
	camera.firstClick = false;
  return camera;
}

void SendCameraMatrix(Camera *camera, GLuint ID, const char* uniform)
{
	// Initializes matrices since otherwise they will be the null matrix
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// Makes camera look in the right direction from the right position
	view = glm::lookAt(camera->position, camera->position + camera->orientation, camera->worldUp);
	// Adds perspective to the scene
	projection = glm::perspective(glm::radians(camera->fov), (float)camera->width / (float)camera->height, 0.1f, 100.0f);

	// Exports the camera matrix to the Vertex Shader
	glUniformMatrix4fv(glGetUniformLocation(ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
}

void ProcessCameraInputs(Camera *camera, GLFWwindow* window)
{
	// Handles key inputs
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera->position += camera->speed * camera->orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera->position += camera->speed * -glm::normalize(glm::cross(camera->orientation, camera->worldUp));
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera->position += camera->speed * -camera->orientation;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera->position += camera->speed * glm::normalize(glm::cross(camera->orientation, camera->worldUp));
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera->position += camera->speed * camera->worldUp;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		camera->position += camera->speed * -camera->worldUp;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera->speed = 0.025f;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
	{
		camera->speed = 0.1f;
	}


	// Handles mouse inputs
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		// Hides mouse cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Prevents camera from jumping on the first click
		if (camera->firstClick)
		{
			glfwSetCursorPos(window, (camera->width / 2), (camera->height / 2));
			camera->firstClick = false;
		}

		// Stores the coordinates of the cursor
		double mouseX;
		double mouseY;
		// Fetches the coordinates of the cursor
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
		// and then "transforms" them into degrees 
		float rotX = camera->sensitivity * (float)(mouseY - (camera->height / 2)) / camera->height;
		float rotY = camera->sensitivity * (float)(mouseX - (camera->width / 2)) / camera->width;

		// Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(camera->orientation, glm::radians(-rotX), glm::normalize(glm::cross(camera->orientation, camera->worldUp)));

		// Decides whether or not the next vertical Orientation is legal or not
		if (abs(glm::angle(newOrientation, camera->worldUp) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			camera->orientation = newOrientation;
		}

		// Rotates the Orientation left and right
		camera->orientation = glm::rotate(camera->orientation, glm::radians(-rotY), camera->worldUp);

		// Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
		glfwSetCursorPos(window, (camera->width / 2), (camera->height / 2));
	}
	else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
	{
		// Unhides cursor since camera is not looking around anymore
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		// Makes sure the next time the camera looks around it doesn't jump
		camera->firstClick = true;
	}
}

void ProcessCameraZoom(Camera *camera, float offset)
{
  camera->fov -= (float)offset;
  if (camera->fov < 1.0f) {
    camera->fov = 1.0f;
  } else if (camera->fov > 45.0f) {
    camera->fov = 45.0f;
  }
	InfoLog("%f",camera->fov);
}