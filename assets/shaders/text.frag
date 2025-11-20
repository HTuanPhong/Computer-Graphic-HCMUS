#version 330 core

in vec2 TexCoord;
in vec4 VertexColor;
out vec4 FragColor;

uniform sampler2D textTexture;

void main() {
    // Sample the ImGui atlas
    vec4 sampled = texture(textTexture, TexCoord);
    
    // Combine vertex color with texture
    // ImGui fonts are white pixels, so we multiply by our desired color
    FragColor = sampled * VertexColor;
}