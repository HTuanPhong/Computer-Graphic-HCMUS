#ifndef APP_H
#define APP_H
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include"Camera.hpp"

struct App
{
	bool shouldCopyPass;
	GLFWwindow* window;
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrameTime = 0.0f;
	int screenWidth;
	int screenHeight;
	Camera camera;
};

extern App g_app; 

bool AppInit(const char *name, int width, int height);
void AppClose();
bool AppRunning();
void AppFrameBegin();
void AppFrameEnd();
void _ErrLog(const char* file, int line, const char* fmt, ...);
void _InfoLog(const char* file, int line, const char* fmt, ...);

#define ErrLog(fmt, ...) _ErrLog(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define InfoLog(fmt, ...) _InfoLog(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif