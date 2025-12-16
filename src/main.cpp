#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "App.hpp"
#include "Render.hpp"
#include "Draw.hpp"
App g_app = {};
namespace ImGui { IMGUI_API void ShowFontAtlas(ImFontAtlas* atlas); }
int main()
{
	if (!AppInit("HCMUS - GEOMETRY APP", 800, 800)) {
		return 1;
	}

	Renderer renderer;
	if (!Renderer_Init(&renderer, 1024)) {
		return 1;
	}
	float projection = 0;
	bool wireFrame = false;

	// Define a light source position in world space
	glm::vec3 lightPos(2.0f, 3.0f, 4.0f);
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 backgroundColor(1.0f, 1.0f, 1.0f);
	int textSize = 14;
	int alpha = 255;

	// In your main setup
	DrawEntities myScene;

	// Add some default data
	// myScene.points.push_back({ {0,0,0}, "Origin" });
	// myScene.points.push_back({ {0,5,0}, "Top" });
	// myScene.points.push_back({ {2,0,0}, "Right" });
	// myScene.cylinders.push_back({ 0, 2, 1 }); // Index 0, 2, 1
	// Main while loop
	while (AppRunning())
	{
		AppFrameBegin();
		Renderer_Begin(&renderer, backgroundColor);
		if (wireFrame) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		// DrawLine(&renderer, {0,0,0}, {0,0,1},{0,0,0,255}, 0.01, 0.05, 0.02);
		// DrawRay(&renderer, {1,0,0}, {1,0,1},{0,0,0,255}, 0.01, 0.05, 0.02);
		// DrawSphere(&renderer, &g_app.camera, {0,0,0}, {0,0,1}, 32, 32, {100,50,100,50}, 
    //             {0,0,0,255}, 0.01, 0.05, 0.02);
		// DrawCircle(&renderer, {0,0,0}, {0,0,1}, {0,1,0}, 64, {200,0,200,255}, {0,0,0,255}, 0.01, 0.05, 0.02);
		// DrawCylinder(&renderer, &g_app.camera,
    //               {0,0,0}, {0,2,0}, {2,0,0}, 
    //               64, {100,0,100,50},
    //               {0,0,0,255}, 0.01, 0.05, 0.02);
		// DrawCone(&renderer, &g_app.camera,
		// 					{0,0,0}, {0,2,0}, {2,0,0}, 
		// 					64, {100,0,100,50},
		// 					{0,0,0,255}, 0.01, 0.05, 0.02);

		// std::vector<glm::vec3> points = {{-4.9,1.08,0}, {-0.87,-3.3,0}, {4.7,-0.04,0}, {1.35,2.98,0}, {-0.93,0.68,0}};
		// DrawSurface(&renderer, points.data(), points.size(), {200,0,200,255}, {0,0,0,255}, 0.01, 0.05, 0.02);
		// DrawPlane(&renderer, {-4.9,1.08,0}, {-0.87,-3.3,0}, {4.7,-0.04,0}, {200,0,200,40});
		// 1. Logic / UI
    DrawGeometryEditor(myScene);

    // 2. Render 3D Scene
    RenderScene(&renderer, &g_app.camera, myScene);

		ImGui::Begin("Settings");
		// ImFontAtlas* atlas = ImGui::GetIO().Fonts;
		// ImGui::ShowFontAtlas(atlas);
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::SliderInt("Text Size", &textSize, 1, 50);
		ImGui::SliderInt("alpha", &alpha, 0, 255);
		ImGui::DragFloat3("Light Position", glm::value_ptr(lightPos), 0.1f);
		ImGui::ColorEdit3("Light Color", glm::value_ptr(lightColor));
		ImGui::ColorEdit3("Background Color", glm::value_ptr(backgroundColor));
		ImGui::Checkbox("Wireframe", &wireFrame);
		if (fabsf(projection - 0.5f) < 0.1f) projection = 0.5f;
		ImGui::SliderFloat("Projection", &projection, 0.0f, 1.0f);
		ImGui::End();


		// Update camera logic
		ProcessOrbitCamera(&g_app.camera, g_app.window);
		SendCameraMatrix(&g_app.camera, renderer.shaderProgram, "camera", projection);
		Renderer_End(&renderer, g_app.camera.position, lightPos, lightColor);
		AppFrameEnd();
	}

	// Cleanup
	Renderer_Destroy(&renderer);
	AppClose();
	return 0;
}