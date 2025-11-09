#include "Render.hpp"
#include "Shader.hpp"

static void CheckShaderCompile(GLuint shader, const char* name);

bool Renderer_Init(Renderer* r, size_t initialVertexCap)
{
  r->capacity = initialVertexCap;
  r->buf.reserve(initialVertexCap);

  glGenVertexArrays(1, &r->vao);
  glBindVertexArray(r->vao);

  glGenBuffers(1, &r->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  glBufferData(GL_ARRAY_BUFFER, r->capacity * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

  GLsizei stride = sizeof(Vertex);
  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, x));

  // Normal attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, nx));

  // Color attribute
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(Vertex, r));

  glBindVertexArray(0);
  if (!CreateShaderProgram("assets/shaders/triangle.vert", "assets/shaders/triangle.frag", &(r->shaderProgram))) {
		return 0;
	}
  return 1;
}

void Renderer_Destroy(Renderer* r)
{
  if (r->vbo) glDeleteBuffers(1, &r->vbo);
  if (r->vao) glDeleteVertexArrays(1, &r->vao);
  if (r->shaderProgram) glDeleteProgram(r->shaderProgram);
  r->vao = r->vbo = r->shaderProgram = 0;
  r->capacity = 0;
  r->buf.clear();
}

void Renderer_Begin(Renderer* r)
{
  // Specify the color of the background
  glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
  // Clean the back buffer and assign the new color to it
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(r->shaderProgram);
  glBindVertexArray(r->vao);
  // global depth test state
  glEnable(GL_DEPTH_TEST);
  r->buf.clear();
}

void Renderer_PushVertices(Renderer* r, const Vertex* verts, size_t count)
{
  r->buf.insert(r->buf.end(), verts, verts + count);
}

void Renderer_PushVertex(Renderer* r, Vertex v)
{
  r->buf.push_back(v);
}

static void EnsureCapacity(Renderer* r, size_t needed)
{
  if (needed <= r->capacity) return;
  size_t newCap = r->capacity ? r->capacity : 1;
  while (newCap < needed) newCap *= 2;
  r->capacity = newCap;

  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  glBufferData(GL_ARRAY_BUFFER, r->capacity * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
}

void Renderer_Flush(Renderer* r)
{
  if (r->buf.empty()) return;
  size_t count = r->buf.size();
  EnsureCapacity(r, count);

  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  glBufferData(GL_ARRAY_BUFFER, r->capacity * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vertex), r->buf.data());

  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
  glBindVertexArray(0);
  glUseProgram(0);

  r->buf.clear();
}

void Renderer_End(Renderer* r)
{
  Renderer_Flush(r);
}