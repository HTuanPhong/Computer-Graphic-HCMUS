#ifndef RENDER_H
#define RENDER_H

#include <glad/glad.h>
#include <vector>
#include <cstddef>

struct Vertex {
  float x, y, z;
  unsigned char r, g, b, a;
};

struct Renderer {
  GLuint vao;
  GLuint vbo;
  size_t capacity;         // vertex capacity
  std::vector<Vertex> buf; // CPU staging buffer
  GLuint shaderProgram;
};

bool Renderer_Init(Renderer* r, size_t initialVertexCap);
void Renderer_Destroy(Renderer* r);
void Renderer_Begin(Renderer* r);
void Renderer_PushVertices(Renderer* r, const Vertex* verts, size_t count);
void Renderer_PushVertex(Renderer* r, Vertex v);
void Renderer_Flush(Renderer* r);
void Renderer_End(Renderer* r);

#endif
