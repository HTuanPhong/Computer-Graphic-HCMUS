#include "App.hpp"
#include <stdio.h>
#include <stdarg.h>
#include"imgui.h"
#include"imgui_internal.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

static void glfwErrorCallback(int error, const char* description)
{
    ErrLog("GLFW %d: %s", error, description);
}

static void ClampAllImGuiWindows()
{
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx)
        return;

    ImGuiViewport* vp = ImGui::GetMainViewport();
    const float padding = 4.0f;

    const ImVec2 clampMin(vp->WorkPos.x + padding, vp->WorkPos.y + padding);
    const ImVec2 clampMax(vp->WorkPos.x + vp->WorkSize.x - padding,
                          vp->WorkPos.y + vp->WorkSize.y - padding);
    const ImVec2 maxSize(vp->WorkSize.x - 2 * padding, vp->WorkSize.y - 2 * padding);

    for (ImGuiWindow* window : ctx->Windows)
    {
        // Skip irrelevant windows
        if (!window->WasActive || window->DockIsActive ||
            (window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)))
            continue;

        ImVec2 pos  = window->Pos;
        ImVec2 size = window->Size;
        bool posChanged  = false;
        bool sizeChanged = false;

        // Detect active resizing (borders or corners)
        bool resizing = (window->ResizeBorderHeld != -1);
        if (!resizing)
        {
            for (int n = 0; n < 2; n++) // check 2 corners
            {
                if (ctx->ActiveId == ImGui::GetWindowResizeCornerID(window, n))
                {
                    resizing = true;
                    break;
                }
            }
        }

        if (resizing)
        {
            // While resizing, clamp the edges being stretched
            if (pos.x < clampMin.x)
            {
                float delta = clampMin.x - pos.x;
                pos.x += delta;
                size.x -= delta;
                posChanged = sizeChanged = true;
            }
            if (pos.y < clampMin.y)
            {
                float delta = clampMin.y - pos.y;
                pos.y += delta;
                size.y -= delta;
                posChanged = sizeChanged = true;
            }
            if (pos.x + size.x > clampMax.x)
            {
                size.x = clampMax.x - pos.x;
                sizeChanged = true;
            }
            if (pos.y + size.y > clampMax.y)
            {
                size.y = clampMax.y - pos.y;
                sizeChanged = true;
            }
        }
        else
        {
            // When not resizing (dragging or static)

            // clamp size to viewport bounds
            if (size.x > maxSize.x) { size.x = maxSize.x; sizeChanged = true; }
            if (size.y > maxSize.y) { size.y = maxSize.y; sizeChanged = true; }

            // clamp position so window stays visible
            if (pos.x < clampMin.x) { pos.x = clampMin.x; posChanged = true; }
            if (pos.y < clampMin.y) { pos.y = clampMin.y; posChanged = true; }
            if (pos.x + size.x > clampMax.x)
            {
                pos.x = clampMax.x - size.x;
                posChanged = true;
            }
            if (pos.y + size.y > clampMax.y)
            {
                pos.y = clampMax.y - size.y;
                posChanged = true;
            }
        }

        // Apply updates
        if (sizeChanged) ImGui::SetWindowSize(window, size);
        if (posChanged)  ImGui::SetWindowPos(window, pos);
    }
}

bool AppInit(const char *name, int width, int height) {
	// unbuff stdout
	setvbuf(stdout, NULL, _IONBF, 0);
  // Initialize GLFW
	glfwInit();
	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// MSAAx8
	glfwWindowHint(GLFW_SAMPLES, 8);
	// Create a GLFWwindow object
	g_app.window = glfwCreateWindow(width, height, name, NULL, NULL);
	// Error check if the window fails to create
	if (g_app.window == NULL)
	{
		ErrLog("Failed to create GLFW window");
		glfwTerminate();
		return 0;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(g_app.window);
  // Enable vsync
  glfwSwapInterval(1);
	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	glViewport(0, 0, width, height);

  glfwSetErrorCallback(glfwErrorCallback);

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
    // no layout save file
	io.IniFilename = nullptr;
    // enable docking
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // load my font
    ImFont* myFont = io.Fonts->AddFontFromFileTTF("assets/fonts/TimesNewRoman.ttf");
    // change font, keep current size
    ImGui::PushFont(myFont, 20.0f);
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(g_app.window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	return 1;
}

void AppClose() {
	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
  // Delete window before ending the program
	glfwDestroyWindow(g_app.window);
	// Terminate GLFW before ending the program
	glfwTerminate();
}

bool AppRunning() {
	return !glfwWindowShouldClose(g_app.window);
}

void AppFrameBegin() {
	// Take care of all GLFW events
	glfwPollEvents();
	// Tell OpenGL a new frame is about to begin
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
}

void AppFrameEnd() {
		ClampAllImGuiWindows();
		// Renders the ImGUI elements
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap the back buffer with the front buffer
		glfwSwapBuffers(g_app.window);
}

void _ErrLog(const char* file, int line, const char* fmt, ...) {
    va_list args;
    fprintf(stderr, "[ERROR][%s:%d] ", file, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void _InfoLog(const char* file, int line, const char* fmt, ...) {
    va_list args;
    fprintf(stderr, "[INFO][%s:%d] ", file, line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}