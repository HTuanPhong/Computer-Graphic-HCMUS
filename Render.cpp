#include "Render.hpp"
#include "Shader.hpp"
#include <algorithm> // Required for std::sort
#include <vector>    // Required for std::vector


struct Triangle {
    Vertex v1, v2, v3;
    float depth;
};

// Comparator function for sorting triangles from back to front
static bool CompareTriangles(const Triangle& a, const Triangle& b) {
    return a.depth > b.depth;
}


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
  
  // Clear the buffer for the new frame
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

void Renderer_Flush(Renderer* r, const Camera& cam)
{
  if (r->buf.empty()) return;

  // --- 1. Segregate Opaque and Transparent Triangles ---
  std::vector<Vertex> opaqueVertices;
  std::vector<Triangle> transparentTriangles;
  const unsigned char OPAQUE_ALPHA_THRESHOLD = 255;

  for (size_t i = 0; i + 2 < r->buf.size(); i += 3) {
      Vertex& v1 = r->buf[i];
      Vertex& v2 = r->buf[i + 1];
      Vertex& v3 = r->buf[i + 2];

      // If all vertices are fully opaque, add to the opaque batch.
      if (v1.a == OPAQUE_ALPHA_THRESHOLD && v2.a == OPAQUE_ALPHA_THRESHOLD && v3.a == OPAQUE_ALPHA_THRESHOLD) {
          opaqueVertices.push_back(v1);
          opaqueVertices.push_back(v2);
          opaqueVertices.push_back(v3);
      } else { // Otherwise, treat the triangle as transparent.
          Triangle tri = {v1, v2, v3};
          // Calculate depth from camera to triangle center (using squared distance for efficiency)
          float centerX = (v1.x + v2.x + v3.x) / 3.0f;
          float centerY = (v1.y + v2.y + v3.y) / 3.0f;
          float centerZ = (v1.z + v2.z + v3.z) / 3.0f;
          float dx = centerX - cam.position.x;
          float dy = centerY - cam.position.y;
          float dz = centerZ - cam.position.z;
          tri.depth = dx*dx + dy*dy + dz*dz;
          transparentTriangles.push_back(tri);
      }
  }

  // Make sure the GPU buffer is large enough for the entire frame's data
  EnsureCapacity(r, r->buf.size());
  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  // --- 2. Opaque Pass ---
  if (!opaqueVertices.empty()) {
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Standard alpha blending
      glDepthMask(GL_TRUE); // Enable depth writing
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writing  
      glDisable(GL_CULL_FACE);

      // Upload and draw opaque vertices
      glBufferSubData(GL_ARRAY_BUFFER, 0, opaqueVertices.size() * sizeof(Vertex), opaqueVertices.data());
      glDrawArrays(GL_TRIANGLES, 0, (GLsizei)opaqueVertices.size());
  }
  // --- 3. Transparent Pass ---
  if (!transparentTriangles.empty()) {
      // Sort transparent triangles from back to front
      // std::sort(transparentTriangles.begin(), transparentTriangles.end(), CompareTriangles);

      // Create a new, sorted vertex buffer for transparent triangles
      std::vector<Vertex> sortedTransparentVertices;
      sortedTransparentVertices.reserve(transparentTriangles.size() * 3);
      for (const auto& tri : transparentTriangles) {
          sortedTransparentVertices.push_back(tri.v1);
          sortedTransparentVertices.push_back(tri.v2);
          sortedTransparentVertices.push_back(tri.v3);
      }
      auto drawAllTransparentObjects = [&]() {
          glBufferSubData(GL_ARRAY_BUFFER, 0, sortedTransparentVertices.size() * sizeof(Vertex), sortedTransparentVertices.data());
          // In GeoGebra, this also does a front/back pass for each object
          glCullFace(GL_FRONT);
          // ... Bind and draw transparent vertices ...
          glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sortedTransparentVertices.size());
          
          glCullFace(GL_BACK);
          // ... Bind and draw transparent vertices ...
          glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sortedTransparentVertices.size());
      };
      // Function to draw objects for the depth-only "hiding" pass
      auto drawHidingTransparentObjects = [&]() {
          glBufferSubData(GL_ARRAY_BUFFER, 0, sortedTransparentVertices.size() * sizeof(Vertex), sortedTransparentVertices.data());
          // This should only draw closed/solid transparent objects, not simple planes
          // ... Bind and draw transparent vertices of CLOSED objects ...
          glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sortedTransparentVertices.size());
      };
      glEnable(GL_CULL_FACE);
     // PASS 1: Color pass
      glDepthMask(GL_FALSE); // Disable depth writing
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writing
      drawAllTransparentObjects();

      // PASS 2: Depth-only pass (back faces)
      glDepthMask(GL_TRUE);  // Enable depth writing
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color writing
      glCullFace(GL_FRONT); // Cull front, draw back
      drawHidingTransparentObjects();

      // PASS 3: Color pass
      glDepthMask(GL_FALSE); // Disable depth writing
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writing
      drawAllTransparentObjects();

      // PASS 4: Depth-only pass (front faces)
      glDepthMask(GL_TRUE); // Enable depth writing
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Disable color writing
      glCullFace(GL_BACK); // Cull back, draw front
      drawHidingTransparentObjects();

      // PASS 5: Color pass
      glDepthMask(GL_FALSE); // Disable depth writing
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writing
      drawAllTransparentObjects();

      glDepthMask(GL_TRUE); // Re-enable depth writing for the next frame
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Re-enable color writing
      glCullFace(GL_BACK);
      glDisable(GL_CULL_FACE);
  }

  r->buf.clear();
}

void Renderer_End(Renderer* r, const Camera& cam)
{
  Renderer_Flush(r, cam);
  
  // --- Restore default OpenGL state ---
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glBindVertexArray(0);
  glUseProgram(0);
}