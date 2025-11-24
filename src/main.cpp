#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<vector>

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
		DrawText(&renderer, &g_app.camera, "A", {-0.5f,-0.5f,-0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "B", {0.5f,-0.5f,-0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "C", {0.5f,-0.5f,0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "D", {-0.5f,-0.5f,0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "A'", {-0.5f,0.5f,-0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "B'", {0.5f,0.5f,-0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "C'", {0.5f,0.5f,0.5f}, textSize, {0,0,0,255});
		DrawText(&renderer, &g_app.camera, "D'", {-0.5f,0.5f,0.5f}, textSize, {0,0,0,255});
		ImVec2 white = ImGui::GetFontTexUvWhitePixel();
		Vertex cube[] = {
			// Front face (Normal: 0, 0, 1)
			{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, white.x,white.y,255,   0,   0, alpha },
			{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, white.x,white.y,255,   0,   0, alpha },
			{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, white.x,white.y,255,   0,   0, alpha },
			{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, white.x,white.y,255,   0,   0, alpha },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, white.x,white.y,255,   0,   0, alpha },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, white.x,white.y,255,   0,   0, alpha },

			// Back face (Normal: 0, 0, -1)
			{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  white.x,white.y,0, 255,   0, alpha },
			{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  white.x,white.y,0, 255,   0, alpha },
			{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  white.x,white.y,0, 255,   0, alpha },
			{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  white.x,white.y,0, 255,   0, alpha },
			{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  white.x,white.y,0, 255,   0, alpha },
			{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  white.x,white.y,0, 255,   0, alpha },

			// Left face (Normal: -1, 0, 0)
			{ -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, white.x,white.y,0,   0, 255, alpha },
			{ -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, white.x,white.y,0,   0, 255, alpha },
			{ -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, white.x,white.y,0,   0, 255, alpha },
			{ -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, white.x,white.y,0,   0, 255, alpha },
			{ -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, white.x,white.y,0,   0, 255, alpha },
			{ -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, white.x,white.y,0,   0, 255, alpha },

			// Right face (Normal: 1, 0, 0)
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, white.x,white.y,255, 255,   0, alpha },
			{  0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, white.x,white.y,255, 255,   0, alpha },
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, white.x,white.y,255, 255,   0, alpha },
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, white.x,white.y,255, 255,   0, alpha },
			{  0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, white.x,white.y,255, 255,   0, alpha },
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, white.x,white.y,255, 255,   0, alpha },

			// Top face (Normal: 0, 1, 0)
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  white.x,white.y,0, 255, 255, alpha },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  white.x,white.y,0, 255, 255, alpha },
			{  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  white.x,white.y,0, 255, 255, alpha },
			{  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  white.x,white.y,0, 255, 255, alpha },
			{  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  white.x,white.y,0, 255, 255, alpha },
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  white.x,white.y,0, 255, 255, alpha },

			// Bottom face (Normal: 0, -1, 0)
			{ -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, white.x,white.y,255,   0, 255, alpha },
			{  0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, white.x,white.y,255,   0, 255, alpha },
			{  0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, white.x,white.y,255,   0, 255, alpha },
			{  0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, white.x,white.y,255,   0, 255, alpha },
			{ -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, white.x,white.y,255,   0, 255, alpha },
			{ -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, white.x,white.y,255,   0, 255, alpha }
		};

		Renderer_PushVertices(&renderer, cube, 36);
		// Sphere properties
		glm::vec3 sphereCenter = {1.0f, 1.0f, 0.0f};
		float sphereRadius = 1.0f;
		int sectorCount = 36;
		int stackCount = 18;
		glm::u8vec4 sphereColor = {255, 0, 0, 255/2};

		DrawSphere(&renderer, sphereCenter, sphereRadius, sectorCount, stackCount, sphereColor);
		DrawSphere(&renderer, {0,0,0}, sphereRadius, sectorCount, stackCount, {255, 255, 0, 255/4});
		DrawSphere(&renderer, {0.0f, 1.0f, 1.0f}, sphereRadius, sectorCount, stackCount, {0, 255, 255, 255/2});
		DrawSphere(&renderer, {0.0f, 2.0f, 0.0f}, sphereRadius, sectorCount, stackCount, {0, 255, 0, 255/2});
		
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