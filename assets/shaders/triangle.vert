#version 330 core
layout (location = 0) in vec3 aPos;     // Vertex position in world space
layout (location = 1) in vec3 aNormal;  // Vertex normal
layout (location = 2) in vec2 aTexCoord;// Vertex texture coordinates
layout (location = 3) in vec4 aColor;   // Vertex color

// Outputs to the fragment shader
out vec3 FragPos;      // Pass-through world position
out vec3 Normal;       // Pass-through normal
out vec2 TexCoord;     // Pass-through texture coordinates
out vec4 VertexColor;  // Pass-through color

// Combined view-projection matrix
uniform mat4 camera;

void main()
{
    FragPos = aPos; // aPos is already in world space, so we pass it directly to the fragment shader.
    Normal = aNormal; // because no model matrix so we pass this through.
    TexCoord = aTexCoord;
    VertexColor = aColor;

    // Calculate the final clip-space position using combined matrix.
    gl_Position = camera * vec4(aPos, 1.0);
}