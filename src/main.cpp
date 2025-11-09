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

/*
 * Creates a circle mesh made of triangles (a triangle fan).
 *
 * @param vertices      - The vector to which the generated vertices will be added.
 * @param center        - The world-space center position of the circle.
 * @param radius        - The radius of the circle.
 * @param num_segments  - The number of segments to use. More segments create a smoother circle.
 * @param color         - The color of the circle.
 */
void CreateCircleMesh(
    std::vector<Vertex>& vertices,
    glm::vec3 center,
    float radius,
    int num_segments,
    glm::u8vec4 color)
{
	// A circle must have at least 3 segments to form a triangle
	if (num_segments < 3) return;

	// The normal for a circle on the XY-plane points up along the Z-axis
	const float normal_x = 0.0f, normal_y = 0.0f, normal_z = 1.0f;

	// Define the center vertex of the fan
	Vertex center_vert = {
		center.x, center.y, center.z,             // Position
		normal_x, normal_y, normal_z,             // Normal
		color.r, color.g, color.b, color.a        // Color
	};

	// Generate a triangle for each segment
	for (int i = 0; i < num_segments; ++i)
	{
		// Calculate the angle for the two points of the segment
		float angle1 = (float)i / (float)num_segments * 2.0f * glm::pi<float>();
		float angle2 = (float)(i + 1) / (float)num_segments * 2.0f * glm::pi<float>();

		// Vertex 1 on the circumference
		Vertex v1 = {
			center.x + radius * cos(angle1), center.y + radius * sin(angle1), center.z,
			normal_x, normal_y, normal_z,
			color.r, color.g, color.b, color.a
		};

		// Vertex 2 on the circumference
		Vertex v2 = {
			center.x + radius * cos(angle2), center.y + radius * sin(angle2), center.z,
			normal_x, normal_y, normal_z,
			color.r, color.g, color.b, color.a
		};

		// Add the triangle (center, v1, v2) to the mesh vector
		vertices.push_back(center_vert);
		vertices.push_back(v1);
		vertices.push_back(v2);
	}
}

/*
 * Creates a sphere mesh made of triangles.
 *
 * @param vertices      - The vector to which the generated vertices will be added.
 * @param center        - The world-space center position of the sphere.
 * @param radius        - The radius of the sphere.
 * @param sectorCount   - The number of longitudinal divisions. Higher is smoother.
 * @param stackCount    - The number of latitudinal divisions. Higher is smoother.
 * @param color         - The color of the sphere.
 */
void CreateSphereMesh(
    std::vector<Vertex>& vertices,
    glm::vec3 center,
    float radius,
    int sectorCount,
    int stackCount,
    glm::u8vec4 color)
{
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = glm::pi<float>() / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = center.z + radius * sinf(stackAngle);   // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = center.x + xy * cosf(sectorAngle);  // r * cos(u) * cos(v)
            y = center.y + xy * sinf(sectorAngle);  // r * cos(u) * sin(v)

            // normalized vertex normal (nx, ny, nz)
            nx = (x - center.x) * lengthInv;
            ny = (y - center.y) * lengthInv;
            nz = (z - center.z) * lengthInv;

            vertices.push_back({ x, y, z, nx, ny, nz, color.r, color.g, color.b, color.a });
        }
    }

    // Now, generate the triangles from the vertices
    std::vector<Vertex> triangleVertices;
    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            if (i != 0)
            {
                triangleVertices.push_back(vertices[k1]);
                triangleVertices.push_back(vertices[k2]);
                triangleVertices.push_back(vertices[k1 + 1]);
            }

            if (i != (stackCount - 1))
            {
                triangleVertices.push_back(vertices[k1 + 1]);
                triangleVertices.push_back(vertices[k2]);
                triangleVertices.push_back(vertices[k2 + 1]);
            }
        }
    }
    // The final list of vertices for drawing
    vertices = triangleVertices;
}

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

	// Define a light source position in world space
	glm::vec3 lightPos(2.0f, 3.0f, 4.0f);
	std::vector<Vertex> meshVertices;
	// Main while loop
	while (AppRunning())
	{
		AppFrameBegin();
		Renderer_Begin(&renderer);

		// A cube with normals for each face.
		// The normals are the second set of three floats (nx, ny, nz).
		Vertex cube[] = {
			// Front face (Normal: 0, 0, 1)
			{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 255,   0,   0, 255 },
			{  0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 255,   0,   0, 255 },
			{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 255,   0,   0, 255 },
			{  0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 255,   0,   0, 255 },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 255,   0,   0, 255 },
			{ -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 255,   0,   0, 255 },

			// Back face (Normal: 0, 0, -1)
			{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0, 255,   0, 255 },
			{  0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0, 255,   0, 255 },
			{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0, 255,   0, 255 },
			{  0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0, 255,   0, 255 },
			{ -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0, 255,   0, 255 },
			{ -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0, 255,   0, 255 },

			// Left face (Normal: -1, 0, 0)
			{ -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0,   0, 255, 255 },
			{ -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0,   0, 255, 255 },
			{ -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0,   0, 255, 255 },
			{ -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0,   0, 255, 255 },
			{ -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0,   0, 255, 255 },
			{ -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0,   0, 255, 255 },

			// Right face (Normal: 1, 0, 0)
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 255, 255,   0, 255 },
			{  0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 255, 255,   0, 255 },
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 255, 255,   0, 255 },
			{  0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 255, 255,   0, 255 },
			{  0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 255, 255,   0, 255 },
			{  0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 255, 255,   0, 255 },

			// Top face (Normal: 0, 1, 0)
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  0, 255, 255, 255 },
			{ -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  0, 255, 255, 255 },
			{  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  0, 255, 255, 255 },
			{  0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,  0, 255, 255, 255 },
			{  0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  0, 255, 255, 255 },
			{ -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  0, 255, 255, 255 },

			// Bottom face (Normal: 0, -1, 0)
			{ -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 255,   0, 255, 255 },
			{  0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 255,   0, 255, 255 },
			{  0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 255,   0, 255, 255 },
			{  0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 255,   0, 255, 255 },
			{ -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 255,   0, 255, 255 },
			{ -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 255,   0, 255, 255 }
		};

		Renderer_PushVertices(&renderer, cube, 36);

		meshVertices.clear();

		// Sphere properties
		glm::vec3 sphereCenter = {1.0f, 1.0f, 0.0f};
		float sphereRadius = 1.0f;
		int sectorCount = 36;
		int stackCount = 18;
		glm::u8vec4 sphereColor = {255, 0, 0, 255}; // Reddish color

		CreateSphereMesh(meshVertices, sphereCenter, sphereRadius, sectorCount, stackCount, sphereColor);

		Renderer_PushVertices(&renderer, meshVertices.data(), meshVertices.size());
		
		ImGui::Begin("Camera");
		ImGui::Checkbox("Ortho", &ortho);
		ImGui::End();

		ImGui::Begin("Light");
		ImGui::DragFloat3("Light Position", glm::value_ptr(lightPos), 0.1f);
		ImGui::End();

		ImGui::Begin("Performance"); 
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::End();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  	

		// Update camera logic
		ProcessOrbitCamera(&g_app.camera, g_app.window);

		// --- EXPORT ALL UNIFORMS TO SHADER ---
		SendCameraMatrix(&g_app.camera, renderer.shaderProgram, "camera", ortho);

		// Get uniform locations
		GLint viewPosLoc = glGetUniformLocation(renderer.shaderProgram, "viewPos");
		GLint lightPosLoc = glGetUniformLocation(renderer.shaderProgram, "lightPos");
		GLint lightColorLoc = glGetUniformLocation(renderer.shaderProgram, "lightColor");

		// Send uniform data to the shader
		glUniform3fv(viewPosLoc, 1, glm::value_ptr(g_app.camera.position));
		glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f); // White light
		// --- END OF UNIFORM EXPORT ---

		Renderer_End(&renderer);
		AppFrameEnd();
	}

	// Cleanup
	Renderer_Destroy(&renderer);
	AppClose();
	return 0;
}