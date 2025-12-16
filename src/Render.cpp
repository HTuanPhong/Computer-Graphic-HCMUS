#include "Render.hpp"
#include "Shader.hpp"
#include "imgui.h"

bool Renderer_Init(Renderer* r, size_t initialVertexCap)
{
  r->capacity = initialVertexCap;
  r->opaqueBuf.reserve(initialVertexCap);
  r->transparentBuf.reserve(initialVertexCap);
  r->hiddenBuf.reserve(initialVertexCap);

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

  // Texcoord attribute
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, u));

  // Color attribute
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(Vertex, r));

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
  r->opaqueBuf.clear();
  r->transparentBuf.clear();
  r->hiddenBuf.clear();
}

void Renderer_Begin(Renderer* r, glm::vec3 backgroundColor)
{
  // Specify the color of the background
  glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
  // Clean the back buffer and assign the new color to it
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // Set global states that are mostly constant
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glFrontFace(GL_CCW); 

  glUseProgram(r->shaderProgram);
  glBindVertexArray(r->vao);
  
  ImFontAtlas* atlas = ImGui::GetIO().Fonts;
  glBindTexture(GL_TEXTURE_2D, atlas->TexRef.GetTexID());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  r->opaqueBuf.clear();
  r->transparentBuf.clear();
  r->hiddenBuf.clear();
}

void Renderer_PushVertex(Renderer* r, Vertex v)
{
    if (v.a == 255) {
        r->opaqueBuf.push_back(v);
    } else if (v.a >= 0) {
        r->transparentBuf.push_back(v);
    }
}

void Renderer_PushHiddenVertex(Renderer* r, Vertex v)
{
    r->hiddenBuf.push_back(v);
}

void Renderer_PushVertices(Renderer* r, const Vertex* verts, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        Renderer_PushVertex(r, verts[i]);
    }
}

static void EnsureCapacity(Renderer* r, size_t needed)
{
  if (needed <= r->capacity) return;
  size_t newCap = r->capacity ? r->capacity : 1;
  while (newCap < needed) newCap *= 2;
  
  r->capacity = newCap;
  glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
  glBufferData(GL_ARRAY_BUFFER, r->capacity * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
  GLsizei stride = sizeof(Vertex);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, x));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, nx));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, u));
  glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)offsetof(Vertex, r));
}

static void DrawBuffer(Renderer* r, const std::vector<Vertex>& buf)
{
    if (buf.empty()) return;
    size_t count = buf.size();
    EnsureCapacity(r, count);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vertex), buf.data());

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

static void DrawTransparentBuffer(Renderer* r) {
    if (r->transparentBuf.empty()) return;

    size_t count = r->transparentBuf.size();
    EnsureCapacity(r, count);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vertex), r->transparentBuf.data());

    // 1. Draw the back-facing triangles
    glCullFace(GL_FRONT);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);

    // 2. Draw the front-facing triangles
    glCullFace(GL_BACK);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

void Renderer_Flush(Renderer* r)
{
  // 1. Draw OPAQUE objects
  glDepthMask(GL_TRUE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDisable(GL_BLEND);
  // glDisable(GL_CULL_FACE);
  glEnable(GL_CULL_FACE); 
  glCullFace(GL_BACK);
  DrawBuffer(r, r->opaqueBuf);

  // 2. Draw TRANSPARENT objects (If any)
  if (!r->transparentBuf.empty()) {
      glDepthMask(GL_FALSE);    
      glEnable(GL_BLEND);       
      glEnable(GL_CULL_FACE);   
      DrawTransparentBuffer(r); 
      glDepthMask(GL_TRUE);     
      
      glEnable(GL_CULL_FACE);   
      glDisable(GL_BLEND);      
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
      glCullFace(GL_FRONT);     
      DrawBuffer(r, r->transparentBuf);
      glDisable(GL_CULL_FACE);  
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
      
      glDepthMask(GL_FALSE);    
      glEnable(GL_BLEND);       
      glEnable(GL_CULL_FACE);   
      DrawTransparentBuffer(r); 
      glDepthMask(GL_TRUE);     
      
      glEnable(GL_CULL_FACE);   
      glDisable(GL_BLEND);      
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
      glCullFace(GL_BACK);     
      DrawBuffer(r, r->transparentBuf);
      glDisable(GL_CULL_FACE);  
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); 
      
      glDepthMask(GL_FALSE);    
      glEnable(GL_BLEND);       
      glEnable(GL_CULL_FACE);   
      DrawTransparentBuffer(r); 
      glDepthMask(GL_TRUE);     
  }
  
  // 3. Draw HIDDEN objects (Opaque, but drawn last)
  if (!r->hiddenBuf.empty()) {
      glDepthMask(GL_TRUE);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glDisable(GL_BLEND);
      glDisable(GL_CULL_FACE);
      
      DrawBuffer(r, r->hiddenBuf);
  }

  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer_End(Renderer* r, glm::vec3 camPos, glm::vec3 lightPos, glm::vec3 lightColor)
{
  GLint viewPosLoc = glGetUniformLocation(r->shaderProgram, "viewPos");
  GLint lightPosLoc = glGetUniformLocation(r->shaderProgram, "lightPos");
  GLint lightColorLoc = glGetUniformLocation(r->shaderProgram, "lightColor");

  glUniform3fv(viewPosLoc, 1, glm::value_ptr(camPos));
  glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
  glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor)); 
  Renderer_Flush(r);
}