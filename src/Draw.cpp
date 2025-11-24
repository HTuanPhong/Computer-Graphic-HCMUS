#include "Draw.hpp"
#include "imgui.h"

void DrawText(Renderer* renderer, Camera* cam, const char* text, glm::vec3 pos, float size, glm::u8vec4 color) {
  if (color.a==255) color.a = 254; // make sure we use transparent path
  ImFont* font = ImGui::GetFont();
  ImFontBaked* baked = font->GetFontBaked(size * 10);
  float finalScale = (size / 10000);
  float offsetX = 0.0f;
  float offsetY = 0.0f;
  for (const char* s = text; *s; s++) {
    ImFontGlyph* glyph = baked->FindGlyph(*s);
    if (!glyph) continue;
    float x1_local = offsetX + glyph->X0 * finalScale;
    float y1_local = offsetY - glyph->Y0 * finalScale; // Inverted Y for 3D
    float x2_local = offsetX + glyph->X1 * finalScale;
    float y2_local = offsetY - glyph->Y1 * finalScale; // Inverted Y for 3D

    glm::vec3 v1 = pos + (cam->camRight * x1_local) + (cam->camUp * y1_local); // Top-Left
    glm::vec3 v2 = pos + (cam->camRight * x2_local) + (cam->camUp * y1_local); // Top-Right
    glm::vec3 v3 = pos + (cam->camRight * x2_local) + (cam->camUp * y2_local); // Bot-Right
    glm::vec3 v4 = pos + (cam->camRight * x1_local) + (cam->camUp * y2_local); // Bot-Left
    float U1 = glyph->U0;
    float V1 = glyph->V0;
    float U2 = glyph->U1;
    float V2 = glyph->V1;
    Renderer_PushVertex(renderer, { v1.x, v1.y, v1.z, cam->camFront.x, cam->camFront.y, cam->camFront.z, U1, V1, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v2.x, v2.y, v2.z, cam->camFront.x, cam->camFront.y, cam->camFront.z, U2, V1, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v3.x, v3.y, v3.z, cam->camFront.x, cam->camFront.y, cam->camFront.z, U2, V2, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v1.x, v1.y, v1.z, cam->camFront.x, cam->camFront.y, cam->camFront.z, U1, V1, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v3.x, v3.y, v3.z, cam->camFront.x, cam->camFront.y, cam->camFront.z, U2, V2, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v4.x, v4.y, v4.z, cam->camFront.x, cam->camFront.y, cam->camFront.z, U1, V2, color.r, color.g, color.b, color.a });
    offsetX += glyph->AdvanceX * finalScale;
  }
}

void DrawSphere(
		Renderer *renderer,
    glm::vec3 center,
    float radius,
    int sectorCount, //- The number of longitudinal divisions. Higher is smoother.
    int stackCount,  //- The number of latitudinal divisions. Higher is smoother.
    glm::u8vec4 color)
{
		std::vector<Vertex> vertices;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal

    float sectorStep = 2 * glm::pi<float>() / sectorCount;
    float stackStep = glm::pi<float>() / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = glm::pi<float>() / 2 - i * stackStep; // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = center.z + radius * sinf(stackAngle);   // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = center.x + xy * cosf(sectorAngle);  // r * cos(u) * cos(v)
            y = center.y + xy * sinf(sectorAngle);  // r * cos(u) * sin(v)

            // normalized vertex normal (nx, ny, nz)
            nx = (x - center.x) * lengthInv;
            ny = (y - center.y) * lengthInv;
            nz = (z - center.z) * lengthInv;
						ImVec2 white = ImGui::GetFontTexUvWhitePixel();
            vertices.push_back({ x, y, z, nx, ny, nz, white.x, white.y, color.r, color.g, color.b, color.a });
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            if (i != 0)
            {
                Renderer_PushVertex(renderer,vertices[k1]);
                Renderer_PushVertex(renderer,vertices[k2]);
                Renderer_PushVertex(renderer,vertices[k1 + 1]);
            }

            if (i != (stackCount - 1))
            {
                Renderer_PushVertex(renderer,vertices[k1 + 1]);
                Renderer_PushVertex(renderer,vertices[k2]);
                Renderer_PushVertex(renderer,vertices[k2 + 1]);
            }
        }
    }
}