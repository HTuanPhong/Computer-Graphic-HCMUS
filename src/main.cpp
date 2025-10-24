#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"

#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include "Shader.h"

int main()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 8);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(800, 800, "HELLO HELLOOOOOOO", NULL, NULL);
	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, 800, 800);
    glEnable(GL_MULTISAMPLE);

	Shader shaderProgram = Shader(
		"shaders/vertex.glsl", "shaders/fragment.glsl",
		"../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl"
	);

	// Vertices coordinates
	GLfloat vertices[] =
	{
		-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower left corner
		0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f, // Lower right corner
		0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f // Upper corner
	};

	// Create reference containers for the Vartex Array Object and the Vertex Buffer Object
	GLuint VAO, VBO;

	// Generate the VAO and VBO with only 1 object each
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// Make the VAO the current Vertex Array Object by binding it
	glBindVertexArray(VAO);

	// Bind the VBO specifying it's a GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Introduce the vertices into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Configure the Vertex Attribute so that OpenGL knows how to read the VBO
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	// Enable the Vertex Attribute so that OpenGL knows to use it
	glEnableVertexAttribArray(0);

	// Bind both the VBO and VAO to 0 so that we don't accidentally modify the VAO and VBO we created
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
    io.IniFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Variables to be changed in the ImGUI window
	bool drawTriangle = true;
	float size = 1.0f;
	float color[4] = { 0.8f, 0.3f, 0.02f, 1.0f };

	// Exporting variables to shaders
	shaderProgram.activateProgram();
	glUniform1f(glGetUniformLocation(shaderProgram.ID, "size"), size);
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "color"), color[0], color[1], color[2], color[3]);

    constexpr float MAX_FPS = 60.f;
    constexpr float FRAME_TIME = 1.f / MAX_FPS;

    float last_time = glfwGetTime();
	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
        float current_time = glfwGetTime();
        float dt = current_time - last_time;
        last_time = current_time;

        if (dt < FRAME_TIME) {
            ImGui_ImplGlfw_Sleep((FRAME_TIME - dt) * 1000);
        }

        // Specify the color of the background
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        // Clean the back buffer and assign the new color to it
        glClear(GL_COLOR_BUFFER_BIT);

        // Tell OpenGL a new frame is about to begin
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
				ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        

        // Tell OpenGL which Shader Program we want to use
        shaderProgram.activateProgram();
        // Bind the VAO so OpenGL knows to use it
        glBindVertexArray(VAO);
        // Only draw the triangle if the ImGUI checkbox is ticked
        if (drawTriangle)
            // Draw the triangle using the GL_TRIANGLES primitive
            glDrawArrays(GL_TRIANGLES, 0, 3);

        // ImGUI window creation
        ImGui::Begin("My name is window, ImGUI window");
        // Text that appears in the window
        ImGui::Text("Hello there adventurer!");
        // Checkbox that appears in the window
        ImGui::Checkbox("Draw Triangle", &drawTriangle);
        // Slider that appears in the window
        ImGui::SliderFloat("Size", &size, 0.5f, 2.0f);
        // Fancy color editor that appears in the window
        ImGui::ColorEdit4("Color", color);
        // Ends the window
        ImGui::End();

        // Export variables to shader
        shaderProgram.activateProgram();
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "size"), size);
        glUniform4f(glGetUniformLocation(shaderProgram.ID, "color"), color[0], color[1], color[2], color[3]);

        // Renders the ImGUI elements
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the back buffer with the front buffer
        glfwSwapBuffers(window);
        // Take care of all GLFW events
        glfwPollEvents();
	}

	// Deletes all ImGUI instances
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Delete all the objects we've created
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	shaderProgram.deleteProgram();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}