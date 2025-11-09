#version 330 core
layout (location = 0) in vec3 aPos;     // Vertex position in world space
layout (location = 1) in vec3 aNormal;  // Vertex normal
layout (location = 2) in vec4 aColor;   // Vertex color

// Outputs to the fragment shader
out vec3 FragPos;      // Pass-through world position
out vec3 Normal;       // Pass-through normal
out vec4 VertexColor;  // Pass-through color

// Your combined view-projection matrix
uniform mat4 camera;

void main()
{
    // aPos is already in world space, so we pass it directly to the fragment shader.
    FragPos = aPos;

    // The normal does not need transformation as there is no model matrix.
    Normal = aNormal;

    // Pass color to the fragment shader.
    VertexColor = aColor;

    // Calculate the final clip-space position using your combined matrix.
    gl_Position = camera * vec4(aPos, 1.0);
}