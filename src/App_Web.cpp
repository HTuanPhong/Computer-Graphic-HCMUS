#include "App.hpp"
#include "Camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include <stdarg.h>
#include <stdio.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

// Error and scroll callbacks (simplified for web)
void errorCallbackglfw(int error, const char *description) {
  ErrLog("GLFW %d: %s", error, description);
}

void framebufferSizeCallbackglfw(GLFWwindow *window, int width, int height) {
  g_app.screenWidth = width;
  g_app.screenHeight = height;
  g_app.camera.width = width;
  g_app.camera.height = height;
  glViewport(0, 0, width, height);
}

void scrollCallbackglfw(GLFWwindow *window, double xoffset, double yoffset) {
  ProcessOrbitZoom(&g_app.camera, (float)yoffset);
}

// Window clamping helper (same as desktop)
static void ClampAllImGuiWindows() {
  ImGuiContext *ctx = ImGui::GetCurrentContext();
  if (!ctx)
    return;
  ImGuiViewport *vp = ImGui::GetMainViewport();
  const float padding = 4.0f;
  const ImVec2 clampMin(vp->WorkPos.x + padding, vp->WorkPos.y + padding);
  const ImVec2 clampMax(vp->WorkPos.x + vp->WorkSize.x - padding,
                        vp->WorkPos.y + vp->WorkSize.y - padding);
  const ImVec2 maxSize(vp->WorkSize.x - 2 * padding,
                       vp->WorkSize.y - 2 * padding);
  for (ImGuiWindow *window : ctx->Windows) {
    if (!window->WasActive || window->DockIsActive ||
        (window->Flags &
         (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)))
      continue;
    ImVec2 pos = window->Pos;
    ImVec2 size = window->Size;
    bool posChanged = false;
    bool sizeChanged = false;
    bool resizing = (window->ResizeBorderHeld != -1);
    if (!resizing) {
      for (int n = 0; n < 2; n++) {
        if (ctx->ActiveId == ImGui::GetWindowResizeCornerID(window, n)) {
          resizing = true;
          break;
        }
      }
    }
    if (resizing) {
      if (pos.x < clampMin.x) {
        float delta = clampMin.x - pos.x;
        pos.x += delta;
        size.x -= delta;
        posChanged = sizeChanged = true;
      }
      if (pos.y < clampMin.y) {
        float delta = clampMin.y - pos.y;
        pos.y += delta;
        size.y -= delta;
        posChanged = sizeChanged = true;
      }
      if (pos.x + size.x > clampMax.x) {
        size.x = clampMax.x - pos.x;
        sizeChanged = true;
      }
      if (pos.y + size.y > clampMax.y) {
        size.y = clampMax.y - pos.y;
        sizeChanged = true;
      }
    } else {
      if (size.x > maxSize.x) {
        size.x = maxSize.x;
        sizeChanged = true;
      }
      if (size.y > maxSize.y) {
        size.y = maxSize.y;
        sizeChanged = true;
      }
      if (pos.x < clampMin.x) {
        pos.x = clampMin.x;
        posChanged = true;
      }
      if (pos.y < clampMin.y) {
        pos.y = clampMin.y;
        posChanged = true;
      }
      if (pos.x + size.x > clampMax.x) {
        pos.x = clampMax.x - size.x;
        posChanged = true;
      }
      if (pos.y + size.y > clampMax.y) {
        pos.y = clampMax.y - size.y;
        posChanged = true;
      }
    }
    if (posChanged)
      window->Pos = pos;
    if (sizeChanged)
      window->Size = size;
  }
}

bool AppInit(const char *name, int width, int height) {
  // Initialize GLFW (Emscripten provides limited GLFW support)
  glfwSetErrorCallback(errorCallbackglfw);

  if (!glfwInit()) {
    ErrLog("Failed to initialize GLFW");
    return 0;
  }

  // Set OpenGL attributes for WebGL2
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create window (for web, this creates a canvas context)
  g_app.window = glfwCreateWindow(width, height, name, nullptr, nullptr);
  if (!g_app.window) {
    ErrLog("Failed to create GLFW window");
    glfwTerminate();
    return 0;
  }

  g_app.screenWidth = width;
  g_app.screenHeight = height;

  // Make the OpenGL context current
  glfwMakeContextCurrent(g_app.window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize ImGUI FIRST - before callbacks (Emscripten may fire callbacks
  // immediately during window creation)
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Set callbacks - now ImGui context exists for handlers
  glfwSetFramebufferSizeCallback(g_app.window, framebufferSizeCallbackglfw);
  glfwSetScrollCallback(g_app.window, scrollCallbackglfw);

  // NOTE: For web (Emscripten), gladLoadGLLoader is not needed.
  // Emscripten provides GL functions directly via WebGL bindings.
  // The GLAD-specific function pointer loading is skipped on web.
  ImGuiIO &io = ImGui::GetIO();
  io.IniFilename = nullptr; // No layout save file on web
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  // Load font - for web, this should be from embedded assets
  ImFont *myFont = nullptr;
  // Try to load from embedded assets
  myFont = io.Fonts->AddFontFromFileTTF("assets/fonts/TimesNewRoman.ttf", 24);
  if (!myFont) {
    // Fallback to default font
    myFont = io.Fonts->AddFontDefault();
  }

  ImGui::PushFont(myFont);
  ImGui::StyleColorsLight();
  ImGui_ImplGlfw_InitForOpenGL(g_app.window, true);
  ImGui_ImplOpenGL3_Init("#version 300 es"); // WebGL2 uses ES3

  g_app.camera =
      CreateOrbitCamera(glm::vec3(0.0f, 0.0f, 0.0f), 5, width, height);
  return 1;
}

void AppClose() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (g_app.window) {
    glfwDestroyWindow(g_app.window);
  }
  glfwTerminate();
}

bool AppRunning() { return !glfwWindowShouldClose(g_app.window); }

void AppFrameBegin() {
  glfwPollEvents(); // Use glfwPollEvents instead of glfwWaitEvents for web

  // Get time using emscripten function
  float currentFrameTime = static_cast<float>(emscripten_get_now() / 1000.0);
  g_app.deltaTime = currentFrameTime - g_app.lastFrameTime;
  g_app.lastFrameTime = currentFrameTime;

  // ImGui frame setup
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void AppFrameEnd() {
  ClampAllImGuiWindows();

  // Render ImGui
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Swap buffers
  glfwSwapBuffers(g_app.window);
}

void _ErrLog(const char *file, int line, const char *fmt, ...) {
  va_list args;
  fprintf(stderr, "[ERROR][%s:%d] ", file, line);
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}

void _InfoLog(const char *file, int line, const char *fmt, ...) {
  va_list args;
  fprintf(stderr, "[INFO][%s:%d] ", file, line);
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
