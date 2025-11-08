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

App g_app = {};

int main()
{
	if (!AppInit("HCMUS - GEOMETRY APP", 800, 800)) {
		return 1;
	}

	Renderer renderer;
	if (!Renderer_Init(&renderer, 1024)) {
		return 1;
	}
	bool ortho = false;

	// Main while loop
	while (AppRunning())
	{
		AppFrameBegin();
		Renderer_Begin(&renderer);
		Vertex cube[] = {
				// Front (red)
				{ -0.5f, -0.5f,  0.5f, 255,   0,   0, 255 },
				{  0.5f, -0.5f,  0.5f, 255,   0,   0, 255 },
				{  0.5f,  0.5f,  0.5f, 255,   0,   0, 255 },
				{  0.5f,  0.5f,  0.5f, 255,   0,   0, 255 },
				{ -0.5f,  0.5f,  0.5f, 255,   0,   0, 255 },
				{ -0.5f, -0.5f,  0.5f, 255,   0,   0, 255 },

				// Back (green)
				{ -0.5f, -0.5f, -0.5f,   0, 255,   0, 255 },
				{  0.5f, -0.5f, -0.5f,   0, 255,   0, 255 },
				{  0.5f,  0.5f, -0.5f,   0, 255,   0, 255 },
				{  0.5f,  0.5f, -0.5f,   0, 255,   0, 255 },
				{ -0.5f,  0.5f, -0.5f,   0, 255,   0, 255 },
				{ -0.5f, -0.5f, -0.5f,   0, 255,   0, 255 },

				// Left (blue)
				{ -0.5f, -0.5f, -0.5f,   0,   0, 255, 255 },
				{ -0.5f, -0.5f,  0.5f,   0,   0, 255, 255 },
				{ -0.5f,  0.5f,  0.5f,   0,   0, 255, 255 },
				{ -0.5f,  0.5f,  0.5f,   0,   0, 255, 255 },
				{ -0.5f,  0.5f, -0.5f,   0,   0, 255, 255 },
				{ -0.5f, -0.5f, -0.5f,   0,   0, 255, 255 },

				// Right (yellow)
				{  0.5f, -0.5f, -0.5f, 255, 255,   0, 255 },
				{  0.5f, -0.5f,  0.5f, 255, 255,   0, 255 },
				{  0.5f,  0.5f,  0.5f, 255, 255,   0, 255 },
				{  0.5f,  0.5f,  0.5f, 255, 255,   0, 255 },
				{  0.5f,  0.5f, -0.5f, 255, 255,   0, 255 },
				{  0.5f, -0.5f, -0.5f, 255, 255,   0, 255 },

				// Top (cyan)
				{ -0.5f,  0.5f, -0.5f,   0, 255, 255, 255 },
				{  0.5f,  0.5f, -0.5f,   0, 255, 255, 255 },
				{  0.5f,  0.5f,  0.5f,   0, 255, 255, 255 },
				{  0.5f,  0.5f,  0.5f,   0, 255, 255, 255 },
				{ -0.5f,  0.5f,  0.5f,   0, 255, 255, 255 },
				{ -0.5f,  0.5f, -0.5f,   0, 255, 255, 255 },

				// Bottom (magenta)
				{ -0.5f, -0.5f, -0.5f, 255,   0, 255, 255 },
				{  0.5f, -0.5f, -0.5f, 255,   0, 255, 255 },
				{  0.5f, -0.5f,  0.5f, 255,   0, 255, 255 },
				{  0.5f, -0.5f,  0.5f, 255,   0, 255, 255 },
				{ -0.5f, -0.5f,  0.5f, 255,   0, 255, 255 },
				{ -0.5f, -0.5f, -0.5f, 255,   0, 255, 255 },
		};

		Renderer_PushVertices(&renderer, cube, 36);

		ImGui::Begin("Vietnamese language test");
		ImGui::Text("“Gạo đem vào giã bao đau đớn,\n"
								"Gạo giã xong rồi, trắng tựa bông.\n"
								"Sống ở trên đời người cũng vậy:\n"
								"Gian nan rèn luyện mới thành công”.\n"
								"                      -Nguyễn Ái Quốc-");
		ImGui::End();

		ImGui::Begin("Camera");
		ImGui::Checkbox("Ortho", &ortho);
		ImGui::End();

		ImGui::Begin("Performance"); 
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::End();

		// Export variables to shader
		ProcessOrbitCamera(&g_app.camera, g_app.window);
		SendCameraMatrix(&g_app.camera, renderer.shaderProgram, "camera", ortho);
		Renderer_End(&renderer);
		AppFrameEnd();
	}

	// Cleanup
	Renderer_Destroy(&renderer);
	AppClose();
	return 0;
}