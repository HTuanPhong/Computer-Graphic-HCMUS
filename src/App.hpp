#include<glad/glad.h>
#include<GLFW/glfw3.h>


struct App
{
	bool shouldCopyPass;
	GLFWwindow* window;
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