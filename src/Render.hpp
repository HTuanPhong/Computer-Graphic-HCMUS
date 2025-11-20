#ifndef RENDER_H
#define RENDER_H

#include <glad/glad.h>
#include <vector>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
  float x, y, z;
  float nx, ny, nz; // Normal vector
  unsigned char r, g, b, a;
};

struct TexVertex {
    float x, y, z;
    float u, v;       // Texture Coordinates
    unsigned char r, g, b, a;
};

struct Renderer {
  GLuint vao;
  GLuint vbo;
  size_t capacity;
  std::vector<Vertex> opaqueBuf; 
  std::vector<Vertex> transparentBuf;
  GLuint shaderProgram;
};

bool Renderer_Init(Renderer* r, size_t initialVertexCap);
void Renderer_Destroy(Renderer* r);
void Renderer_Begin(Renderer* r);
void Renderer_PushVertices(Renderer* r, const Vertex* verts, size_t count);
void Renderer_PushVertex(Renderer* r, Vertex v);
void Renderer_Flush(Renderer* r);
void Renderer_End(Renderer* r, glm::vec3 camPos, glm::vec3 lightPos, glm::vec3 lightColor);

#endif