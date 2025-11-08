#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

// Passed to the fragment shader
out vec4 vColor;

uniform mat4 camera;
void main()
{
  vColor = aColor;
  gl_Position = camera * vec4(aPos, 1.0);
}