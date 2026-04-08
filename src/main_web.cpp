#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
// For web, don't include glad.h - Emscripten provides GL functions directly via
// WebGL/GLES3 #include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include <emscripten/emscripten.h>

#include "App.hpp"
#include "Draw.hpp"
#include "Render.hpp"

// Forward declaration
void SetWebScenePointer(DrawEntities *entities);

App g_app = {};
namespace ImGui {
IMGUI_API void ShowFontAtlas(ImFontAtlas *atlas);
}

// Global state for the main loop callback
struct WebAppState {
  Renderer *renderer;
  DrawEntities *scene;
  float lightPos[3] = {2.0f, 3.0f, 4.0f};
  float lightColor[3] = {1.0f, 1.0f, 1.0f};
  float backgroundColor[3] = {1.0f, 1.0f, 1.0f};
  int textSize = 14;
  int alpha = 255;
  float projection = 0.0f;
  bool wireFrame = false;
} g_webAppState;

// Main loop callback for Emscripten
void MainLoopWeb() {
  WebAppState *state = &g_webAppState;
  Renderer *renderer = state->renderer;
  DrawEntities *scene = state->scene;

  AppFrameBegin();
  Renderer_Begin(renderer,
                 glm::vec3(state->backgroundColor[0], state->backgroundColor[1],
                           state->backgroundColor[2]));

// Note: glPolygonMode doesn't exist in WebGL/GLES3
#ifndef __EMSCRIPTEN__
  if (state->wireFrame) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
#endif

  // Logic / UI
  DrawGeometryEditor(*scene);

  // Render 3D Scene
  RenderScene(renderer, &g_app.camera, *scene);

  // Settings Panel
  ImGui::Begin("Settings");
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  ImGui::DragFloat3("Light Position", state->lightPos, 0.1f);
  ImGui::ColorEdit3("Light Color", state->lightColor);
  ImGui::ColorEdit3("Background Color", state->backgroundColor);
  ImGui::Checkbox("Wireframe", &state->wireFrame);
  if (fabsf(state->projection - 0.5f) < 0.1f)
    state->projection = 0.5f;
  ImGui::SliderFloat("Projection", &state->projection, 0.0f, 1.0f);
  ImGui::End();

  // Update camera logic
  ProcessOrbitCamera(&g_app.camera, g_app.window);
  SendCameraMatrix(&g_app.camera, renderer->shaderProgram, "camera",
                   state->projection);
  Renderer_End(
      renderer, g_app.camera.position,
      glm::vec3(state->lightPos[0], state->lightPos[1], state->lightPos[2]),
      glm::vec3(state->lightColor[0], state->lightColor[1],
                state->lightColor[2]));
  AppFrameEnd();
}

int main() {
  if (!AppInit("HCMUS - GEOMETRY APP", 800, 800)) {
    return 1;
  }

  Renderer renderer;
  if (!Renderer_Init(&renderer, 1024)) {
    return 1;
  }

  // Setup web app state
  g_webAppState.renderer = &renderer;
  g_webAppState.scene = new DrawEntities();

// Set web scene pointer for file loading
#ifdef __EMSCRIPTEN__
  SetWebScenePointer(g_webAppState.scene);
#endif

  // Install Emscripten callbacks for event handling
  ImGui_ImplGlfw_InstallEmscriptenCallbacks(g_app.window, "#canvas");

  // Set up the main loop using Emscripten's scheduling
  emscripten_set_main_loop(MainLoopWeb, 0, true);

  // Cleanup (will not be reached in normal Emscripten execution)
  Renderer_Destroy(&renderer);
  delete g_webAppState.scene;
  AppClose();
  return 0;
}
