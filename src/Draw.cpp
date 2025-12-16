#include "Draw.hpp"
#include "imgui.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>

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
    Renderer_PushVertex(renderer, { v1.x, v1.y, v1.z, cam->camRight.x, cam->camRight.y, cam->camRight.z, U1, V1, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v2.x, v2.y, v2.z, cam->camRight.x, cam->camRight.y, cam->camRight.z, U2, V1, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v3.x, v3.y, v3.z, cam->camRight.x, cam->camRight.y, cam->camRight.z, U2, V2, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v1.x, v1.y, v1.z, cam->camRight.x, cam->camRight.y, cam->camRight.z, U1, V1, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v3.x, v3.y, v3.z, cam->camRight.x, cam->camRight.y, cam->camRight.z, U2, V2, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { v4.x, v4.y, v4.z, cam->camRight.x, cam->camRight.y, cam->camRight.z, U1, V2, color.r, color.g, color.b, color.a });
    offsetX += glyph->AdvanceX * finalScale;
  }
}

struct PipeNode {
    glm::vec3 position;
    glm::vec3 normal; // We will approximate normal for lighting
};
// Helper to find intersection between a line (Point + Dir) and a Plane (Point + Normal)
// Based on the formula: t = ((PlanePoint - LineStart) . PlaneNormal) / (LineDir . PlaneNormal)
static glm::vec3 IntersectLinePlane(glm::vec3 lineStart, glm::vec3 lineDir, glm::vec3 planePoint, glm::vec3 planeNormal) {
    float denom = glm::dot(lineDir, planeNormal);
    
    // Prevent division by zero if line is perfectly parallel to plane 
    // (Should be rare in a valid pipe path, but good for safety)
    if (glm::abs(denom) < 1e-5f) {
        return lineStart; 
    }

    float t = glm::dot(planePoint - lineStart, planeNormal) / denom;
    return lineStart + (lineDir * t);
}

static void DrawCap(Renderer* renderer, glm::vec3 center, const std::vector<PipeNode>& ring, glm::vec3 normal, glm::u8vec4 color, bool isStartCap) {
    // ImVec2 white = ImGui::GetFontTexUvWhitePixel();
    // size_t segments = ring.size();

    // for (size_t j = 0; j < segments; j++) {
    //     size_t next = (j + 1) % segments;

    //     // Vertices
    //     glm::vec3 vCenter = center;
    //     glm::vec3 vCurrent = ring[j].position;
    //     glm::vec3 vNext = ring[next].position;

    //     // If it's a start cap, we look at it from behind, so we swap winding order
    //     // so the triangle faces the camera.
    //     if (isStartCap) {
    //         std::swap(vCurrent, vNext);
    //     }

    //     Renderer_PushVertex(renderer, { vCenter.x, vCenter.y, vCenter.z, normal.x, normal.y, normal.z, white.x, white.y, color.r, color.g, color.b, color.a });
    //     Renderer_PushVertex(renderer, { vCurrent.x, vCurrent.y, vCurrent.z, normal.x, normal.y, normal.z, white.x, white.y, color.r, color.g, color.b, color.a });
    //     Renderer_PushVertex(renderer, { vNext.x, vNext.y, vNext.z, normal.x, normal.y, normal.z, white.x, white.y, color.r, color.g, color.b, color.a });
    // }
    return;
}

static void DrawHiddenSegment(Renderer* renderer, glm::vec3 *pos, int posSize, float radius, glm::u8vec4 color) {
    if (posSize < 2 || pos == nullptr || renderer == nullptr) return;

    const int SEGMENTS = 12; 
    const float PI = glm::pi<float>();
    ImVec2 white = ImGui::GetFontTexUvWhitePixel();


    std::vector<PipeNode> prevRing(SEGMENTS);
    std::vector<PipeNode> currRing(SEGMENTS);

    // --- STEP 1: Generate the very first ring at pos[0] ---
    // We only generate a circle from scratch ONCE.
    {
        glm::vec3 forward = glm::normalize(pos[1] - pos[0]);
        
        // Calculate stable basis vectors
        glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(forward, globalUp)) > 0.99f) globalUp = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 right = glm::normalize(glm::cross(forward, globalUp));
        glm::vec3 up    = glm::normalize(glm::cross(right, forward));

        for (int j = 0; j < SEGMENTS; j++) {
            float theta = (float)j / (float)SEGMENTS * 2.0f * PI;
            float cosTheta = cos(theta);
            float sinTheta = sin(theta);
            
            // Circle on the plane perpendicular to the start
            glm::vec3 localPos = (right * cosTheta + up * sinTheta) * radius;
            prevRing[j].position = pos[0] + localPos;
            prevRing[j].normal   = glm::normalize(localPos); 
        }
        DrawCap(renderer, pos[0], prevRing, -forward, color, true);
    }

    // --- STEP 2: Extrude along the path ---
    for (int i = 1; i < posSize; i++) {
        glm::vec3 Q_prev = pos[i - 1]; // Q1
        glm::vec3 Q_curr = pos[i];     // Q2
        
        // v1 = Q2 - Q1 (Direction of the current segment)
        glm::vec3 v1 = glm::normalize(Q_curr - Q_prev);
        
        // Calculate the Plane Normal (n) at Q_curr
        glm::vec3 planeNormal;
        
        if (i < posSize - 1) {
            // Turning Point (Miter Joint)
            glm::vec3 Q_next = pos[i + 1]; // Q3
            glm::vec3 v2 = glm::normalize(Q_next - Q_curr); // Q3 - Q2
            
            // n = normalize(v1 + v2)  <-- As per your image formula
            planeNormal = glm::normalize(v1 + v2);
        } else {
            // End Point (Cap)
            // No turn, so the plane is just perpendicular to the last segment
            planeNormal = v1;
        }

        // --- STEP 3: Project points from Previous Ring to Current Ring ---
        for (int j = 0; j < SEGMENTS; j++) {
            glm::vec3 P_prev = prevRing[j].position;

            // Math from image:
            // Find P' (currRing pos) by intersecting the line from P_prev with direction v1
            // against the plane defined by (Point: Q_curr, Normal: planeNormal).
            
            currRing[j].position = IntersectLinePlane(P_prev, v1, Q_curr, planeNormal);

            // Recalculate normal for lighting
            // The normal is simply the vector from the center line to the surface
            currRing[j].normal = glm::normalize(currRing[j].position - Q_curr);
        }

        // --- STEP 4: Draw Triangles ---
        for (int j = 0; j < SEGMENTS; j++) {
            int nextJ = (j + 1) % SEGMENTS; // Wrap around

            PipeNode& p0 = prevRing[j];
            PipeNode& p1 = prevRing[nextJ];
            PipeNode& c0 = currRing[j];
            PipeNode& c1 = currRing[nextJ];

            // Triangle 1: p0 -> c0 -> p1
            Renderer_PushHiddenVertex(renderer, {
                p0.position.x, p0.position.y, p0.position.z,
                p0.normal.x, p0.normal.y, p0.normal.z,
                white.x, white.y,
                color.r, color.g, color.b, color.a
            });
            Renderer_PushHiddenVertex(renderer, {
                c0.position.x, c0.position.y, c0.position.z,
                c0.normal.x, c0.normal.y, c0.normal.z,
                white.x, white.y,
                color.r, color.g, color.b, color.a
            });
            Renderer_PushHiddenVertex(renderer, {
                p1.position.x, p1.position.y, p1.position.z,
                p1.normal.x, p1.normal.y, p1.normal.z,
                white.x, white.y,
                color.r, color.g, color.b, color.a
            });

            // Triangle 2: p1 -> c0 -> c1
            Renderer_PushHiddenVertex(renderer, {
                p1.position.x, p1.position.y, p1.position.z,
                p1.normal.x, p1.normal.y, p1.normal.z,
                white.x, white.y,
                color.r, color.g, color.b, color.a
            });
            Renderer_PushHiddenVertex(renderer, {
                c0.position.x, c0.position.y, c0.position.z,
                c0.normal.x, c0.normal.y, c0.normal.z,
                white.x, white.y,
                color.r, color.g, color.b, color.a
            });
            Renderer_PushHiddenVertex(renderer, {
                c1.position.x, c1.position.y, c1.position.z,
                c1.normal.x, c1.normal.y, c1.normal.z,
                white.x, white.y,
                color.r, color.g, color.b, color.a
            });
        }
        if (i == posSize - 1) {
            DrawCap(renderer, pos[i], currRing, v1, color, false);
        }
        // Advance: The current ring becomes the previous ring for the next segment
        prevRing = currRing;
    }
}

void DrawSegment(Renderer* renderer, glm::vec3* pos, int posSize, float radius, glm::u8vec4 color, float dashLen, float gapLen) {
    if (posSize < 2 || !pos || !renderer) return;
    if (color.a == 0) return;
    if (dashLen <= 0.0f) dashLen = 0.1f;

    DrawHiddenSegment(renderer, pos, posSize, radius,color);
    radius /= 2;
    const int SEGMENTS = 12;
    const float PI = glm::pi<float>();
    ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
    
    // Pattern state
    float patternLen = dashLen + gapLen;
    float currentPatternPos = 0.0f; 

    // Pre-allocate vectors to avoid recreating them in loops
    std::vector<PipeNode> startRing(SEGMENTS);
    std::vector<PipeNode> endRing(SEGMENTS);
    
    // Pre-calculate Unit Circle (Cos/Sin)
    std::vector<glm::vec2> circleLUT(SEGMENTS);
    for(int j=0; j<SEGMENTS; j++) {
        float theta = (float)j / SEGMENTS * 2.0f * PI;
        circleLUT[j] = { cos(theta), sin(theta) };
    }

    for (int i = 0; i < posSize - 1; i++) {
        glm::vec3 p1 = pos[i];
        glm::vec3 p2 = pos[i + 1];
        glm::vec3 dir = p2 - p1;
        float segLen = glm::length(dir);
        
        if (segLen < 1e-5f) continue;
        dir /= segLen; // Normalize

        // Calculate Basis (Right/Up) for this segment
        glm::vec3 globalUp = (glm::abs(dir.y) > 0.99f) ? glm::vec3(0,0,1) : glm::vec3(0,1,0);
        glm::vec3 right = glm::normalize(glm::cross(dir, globalUp));
        glm::vec3 up    = glm::normalize(glm::cross(right, dir));

        float distTraveled = 0.0f;

        // Walk along the segment
        while (distTraveled < segLen) {
            float distRemaining = segLen - distTraveled;
            bool isDash = currentPatternPos < dashLen;
            float distToNextState = isDash ? (dashLen - currentPatternPos) : (patternLen - currentPatternPos);
            
            // Step size is limited by segment length OR dash/gap length
            float step = glm::min(distRemaining, distToNextState);

            if (isDash) {
                glm::vec3 startCenter = p1 + dir * distTraveled;
                glm::vec3 endCenter   = startCenter + dir * step;

                // Build rings for this specific pellet
                for (int j = 0; j < SEGMENTS; j++) {
                    glm::vec3 offset = (right * circleLUT[j].x + up * circleLUT[j].y) * radius;
                    glm::vec3 normal = glm::normalize(offset); // Normal points out from center
                    
                    startRing[j] = { startCenter + offset, normal };
                    endRing[j]   = { endCenter + offset,   normal };
                }

                // A. Draw Cylinder Body
                for (int j = 0; j < SEGMENTS; j++) {
                    int next = (j + 1) % SEGMENTS;
                    PipeNode& s0 = startRing[j]; PipeNode& s1 = startRing[next];
                    PipeNode& e0 = endRing[j];   PipeNode& e1 = endRing[next];

                    Renderer_PushVertex(renderer, { s0.position.x, s0.position.y, s0.position.z, s0.normal.x, s0.normal.y, s0.normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
                    Renderer_PushVertex(renderer, { e0.position.x, e0.position.y, e0.position.z, e0.normal.x, e0.normal.y, e0.normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
                    Renderer_PushVertex(renderer, { s1.position.x, s1.position.y, s1.position.z, s1.normal.x, s1.normal.y, s1.normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });

                    Renderer_PushVertex(renderer, { s1.position.x, s1.position.y, s1.position.z, s1.normal.x, s1.normal.y, s1.normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
                    Renderer_PushVertex(renderer, { e0.position.x, e0.position.y, e0.position.z, e0.normal.x, e0.normal.y, e0.normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
                    Renderer_PushVertex(renderer, { e1.position.x, e1.position.y, e1.position.z, e1.normal.x, e1.normal.y, e1.normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
                }

                // B. Draw Caps
                DrawCap(renderer, startCenter, startRing, -dir, color, true);
                DrawCap(renderer, endCenter,   endRing,    dir, color, false);
            }

            // Advance
            distTraveled += step;
            currentPatternPos += step;
            if (currentPatternPos >= patternLen) currentPatternPos -= patternLen;
        }
    }
}

void DrawAngle(Renderer* renderer, const Camera* cam, 
               glm::vec3 start, glm::vec3 vertex, glm::vec3 end, 
               glm::u8vec4 color, float radius, float thickness, 
               float textSize, float textOffset) 
{
    if (!renderer || !cam) return;

    glm::vec3 v1 = start - vertex;
    glm::vec3 v2 = end - vertex;

    if (glm::length(v1) < 0.001f || glm::length(v2) < 0.001f) return;

    v1 = glm::normalize(v1);
    v2 = glm::normalize(v2);

    float angleRad = glm::angle(v1, v2);
    float angleDeg = glm::degrees(angleRad);

    if (angleDeg < 0.1f) return;

    // Check for Right Angle (90 degrees +/- tolerance)
    if (std::abs(angleDeg - 90.0f) < 0.5f) {
        // --- DRAW RIGHT ANGLE SYMBOL (Square Corner) ---
        
        // We need a smaller radius for the square symbol usually, or use the provided radius
        // Let's use the provided radius directly.
        
        // Points on the legs
        glm::vec3 p1 = vertex + v1 * radius;
        glm::vec3 p2 = vertex + v2 * radius;
        
        // The corner point completes the parallelogram (square in this case)
        // Corner = Vertex + v1*r + v2*r
        glm::vec3 corner = vertex + (v1 + v2) * radius;

        // Draw 2 segments: P1->Corner->P2
        glm::vec3 strip[] = { p1, corner, p2 };
        DrawSegment(renderer, strip, 3, thickness, color, 0.0f, 0.0f);

    } else {
        // --- DRAW ARC ---
        const int STEPS = 24;
        std::vector<glm::vec3> arcPoints;
        arcPoints.reserve(STEPS + 1);

        glm::vec3 normal = glm::cross(v1, v2);
        if (glm::length(normal) < 0.001f) normal = glm::vec3(0,1,0); 
        else normal = glm::normalize(normal);

        glm::vec3 side = glm::normalize(glm::cross(normal, v1));

        for (int i = 0; i <= STEPS; i++) {
            float t = (float)i / (float)STEPS;
            float theta = angleRad * t;
            glm::vec3 p = vertex + (v1 * cos(theta) + side * sin(theta)) * radius;
            arcPoints.push_back(p);
        }

        DrawSegment(renderer, arcPoints.data(), (int)arcPoints.size(), thickness, color, 0.0f, 0.0f);
    }

    // --- DRAW TEXT ---
    // Position text at the bisector of the angle
    glm::vec3 midDir = glm::normalize(v1 + v2);
    
    // Base position is slightly outside the arc/square
    glm::vec3 textPos = vertex + midDir * (radius + 0.2f);
    
    // Apply Global Y Offset
    textPos.y += textOffset;
    
    char buf[32];
    snprintf(buf, 32, "%.1f deg", angleDeg);
    
    // Use the declared global DrawText
    DrawText(renderer, (Camera*)cam, buf, textPos, textSize, {0, 0, 0, 255});
}

static const float INF_LENGTH = 100.0f; 

// Ray: Starts at 'start', goes through 'passThrough' to infinity
void DrawRay(Renderer* renderer, glm::vec3 start, glm::vec3 passThrough, 
             glm::u8vec4 color, float thickness, float dashLen, float gapLen) 
{
    if (!renderer) return;

    glm::vec3 dir = passThrough - start;
    float len = glm::length(dir);
    if (len < 1e-5f) return;
    dir = dir / len;

    glm::vec3 end = start + (dir * INF_LENGTH);

    glm::vec3 points[2] = { start, end };
    DrawSegment(renderer, points, 2, thickness, color, dashLen, gapLen);
}

// Line: Infinite in both directions passing through p1 and p2
void DrawLine(Renderer* renderer, glm::vec3 p1, glm::vec3 p2, 
                glm::u8vec4 color, float thickness, float dashLen, float gapLen) 
{
    if (!renderer) return;

    glm::vec3 dir = p2 - p1;
    float len = glm::length(dir);
    if (len < 1e-5f) return;
    dir = dir / len;

    glm::vec3 start = p1 - (dir * INF_LENGTH);
    glm::vec3 end   = p1 + (dir * INF_LENGTH);

    glm::vec3 points[2] = { start, end };
    DrawSegment(renderer, points, 2, thickness, color, dashLen, gapLen);
}

// Helper: Check if point P is inside triangle ABC (2D)
static bool IsPointInTriangle(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) {
    auto crossProduct = [](const glm::vec2& a, const glm::vec2& b) {
        return a.x * b.y - a.y * b.x;
    };
    
    // Check signs of cross product for P relative to edges AB, BC, CA
    float cp1 = crossProduct(b - a, p - a);
    float cp2 = crossProduct(c - b, p - b);
    float cp3 = crossProduct(a - c, p - c);

    // If all have the same sign (and are not zero), the point is inside
    bool has_neg = (cp1 < 0) || (cp2 < 0) || (cp3 < 0);
    bool has_pos = (cp1 > 0) || (cp2 > 0) || (cp3 > 0);
    return !(has_neg && has_pos);
}

void DrawSurface(Renderer* renderer, glm::vec3* points, int count, 
                 glm::u8vec4 surfaceColor, 
                 glm::u8vec4 borderColor, float thickness, float dashLen, float gapLen)
{
    if (!renderer || !points || count < 3) return;

    // --- STEP 1: Check Coplanarity & Get Projection Axis ---
    glm::vec3 avgNormal(0.0f);
    for (int i = 0; i < count; i++) {
        glm::vec3 curr = points[i];
        glm::vec3 next = points[(i + 1) % count];
        avgNormal.x += (curr.y - next.y) * (curr.z + next.z);
        avgNormal.y += (curr.z - next.z) * (curr.x + next.x);
        avgNormal.z += (curr.x - next.x) * (curr.y + next.y);
    }
    
    bool isValidSurface = true;
    if (glm::length(avgNormal) < 1e-5f) {
        isValidSurface = false;
    } else {
        avgNormal = glm::normalize(avgNormal);
        float planeD = -glm::dot(avgNormal, points[0]);
        const float TOLERANCE = 0.01f; 

        for (int i = 1; i < count; i++) {
            float dist = glm::abs(glm::dot(avgNormal, points[i]) + planeD);
            if (dist > TOLERANCE) {
                isValidSurface = false;
                break;
            }
        }
    }

    // --- STEP 2: Draw Filled Surface (Double Sided) ---
    if (isValidSurface && 1) {
        ImVec2 uv = ImGui::GetFontTexUvWhitePixel();

        // 1. Project to 2D
        int dropAxis = 0; 
        float maxN = glm::abs(avgNormal.x);
        if (glm::abs(avgNormal.y) > maxN) { maxN = glm::abs(avgNormal.y); dropAxis = 1; }
        if (glm::abs(avgNormal.z) > maxN) { maxN = glm::abs(avgNormal.z); dropAxis = 2; }

        std::vector<glm::vec2> poly2D(count);
        for (int i = 0; i < count; i++) {
            if (dropAxis == 0)      poly2D[i] = { points[i].y, points[i].z };
            else if (dropAxis == 1) poly2D[i] = { points[i].z, points[i].x };
            else                    poly2D[i] = { points[i].x, points[i].y };
        }

        // 2. Determine Winding
        float area = 0.0f;
        for (int i = 0; i < count; i++) {
            glm::vec2 curr = poly2D[i];
            glm::vec2 next = poly2D[(i + 1) % count];
            area += (next.x - curr.x) * (next.y + curr.y);
        }
        bool isCCW = (area < 0.0f);

        // 3. Triangulate
        std::vector<int> indices(count);
        for (int i = 0; i < count; i++) indices[i] = i;

        int safety = count * 2;
        while (indices.size() > 2 && safety-- > 0) {
            bool earFound = false;
            size_t n = indices.size();

            for (size_t i = 0; i < n; i++) {
                int iPrev = indices[(i + n - 1) % n];
                int iCurr = indices[i];
                int iNext = indices[(i + 1) % n];

                glm::vec2 a = poly2D[iPrev];
                glm::vec2 b = poly2D[iCurr];
                glm::vec2 c = poly2D[iNext];

                glm::vec2 ab = b - a;
                glm::vec2 bc = c - b;
                float cross = ab.x * bc.y - ab.y * bc.x;
                bool isConvex = isCCW ? (cross > 1e-6f) : (cross < -1e-6f);

                if (isConvex) {
                    bool empty = true;
                    for (size_t j = 0; j < n; j++) {
                        int other = indices[j];
                        if (other == iPrev || other == iCurr || other == iNext) continue;
                        if (IsPointInTriangle(poly2D[other], a, b, c)) {
                            empty = false;
                            break;
                        }
                    }

                    if (empty) {
                        glm::vec3 p0 = points[iPrev];
                        glm::vec3 p1 = points[iCurr];
                        glm::vec3 p2 = points[iNext];

                        // Calc Triangle Normal
                        glm::vec3 v1 = p1 - p0;
                        glm::vec3 v2 = p2 - p0;
                        glm::vec3 triNormal = glm::cross(v1, v2);
                        
                        if (glm::length(triNormal) > 1e-6f) {
                            triNormal = glm::normalize(triNormal);

                            // Front
                            Renderer_PushVertex(renderer, { p0.x, p0.y, p0.z, triNormal.x, triNormal.y, triNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
                            Renderer_PushVertex(renderer, { p1.x, p1.y, p1.z, triNormal.x, triNormal.y, triNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
                            Renderer_PushVertex(renderer, { p2.x, p2.y, p2.z, triNormal.x, triNormal.y, triNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });

                            // Back
                            glm::vec3 invNormal = -triNormal;
                            Renderer_PushVertex(renderer, { p0.x, p0.y, p0.z, invNormal.x, invNormal.y, invNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
                            Renderer_PushVertex(renderer, { p2.x, p2.y, p2.z, invNormal.x, invNormal.y, invNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
                            Renderer_PushVertex(renderer, { p1.x, p1.y, p1.z, invNormal.x, invNormal.y, invNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
                        }

                        indices.erase(indices.begin() + i);
                        earFound = true;
                        break;
                    }
                }
            }
            if (!earFound) break;
        }
    }

    // --- STEP 3: Draw Perimeter (Fixed for Dashes) ---
    if (borderColor.a > 0 && thickness > 0.0f) {
        for (int i = 0; i < count; i++) {
            glm::vec3 p1 = points[i];
            glm::vec3 p2 = points[(i + 1) % count];

            // Canonical Sort: Ensure drawing direction is always deterministic
            // based on coordinates (e.g. Small X -> Large X).
            // This ensures adjacent faces draw the dash pattern starting from the same vertex.
            bool swap = false;
            if (p1.x > p2.x) swap = true;
            else if (std::abs(p1.x - p2.x) < 1e-5f) {
                if (p1.y > p2.y) swap = true;
                else if (std::abs(p1.y - p2.y) < 1e-5f) {
                    if (p1.z > p2.z) swap = true;
                }
            }

            glm::vec3 edge[2];
            if (swap) {
                edge[0] = p2; 
                edge[1] = p1;
            } else {
                edge[0] = p1;
                edge[1] = p2;
            }

            DrawSegment(renderer, edge, 2, thickness, borderColor, dashLen, gapLen);
        }
    }
}

void DrawPlane(Renderer* renderer, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::u8vec4 color) 
{
    if (!renderer) return;

    // 1. Calculate Plane Normal
    glm::vec3 v1 = p2 - p1;
    glm::vec3 v2 = p3 - p1;
    glm::vec3 normal = glm::cross(v1, v2);

    float len = glm::length(normal);
    if (len < 1e-5f) return; // Points are collinear, cannot define a plane
    normal /= len;

    // 2. Create Basis Vectors on the Plane
    // U axis: Direction from p1 to p2
    glm::vec3 u = glm::normalize(v1);
    // V axis: Perpendicular to U and Normal
    glm::vec3 v = glm::cross(normal, u);

    // 3. Generate 4 Corners at "Infinity"
    // We extend the plane by a large amount from the center anchor (p1)
    const float INF = 10000.0f; // Large enough to cover the view

    glm::vec3 c1 = p1 + (u * INF) + (v * INF);
    glm::vec3 c2 = p1 - (u * INF) + (v * INF);
    glm::vec3 c3 = p1 - (u * INF) - (v * INF);
    glm::vec3 c4 = p1 + (u * INF) - (v * INF);

    // 4. Draw Quad (2 Triangles)
    ImVec2 uv = ImGui::GetFontTexUvWhitePixel();

    // Triangle 1: c1 -> c2 -> c3
    Renderer_PushVertex(renderer, { c1.x, c1.y, c1.z, normal.x, normal.y, normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { c2.x, c2.y, c2.z, normal.x, normal.y, normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { c3.x, c3.y, c3.z, normal.x, normal.y, normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });

    // Triangle 2: c1 -> c3 -> c4
    Renderer_PushVertex(renderer, { c1.x, c1.y, c1.z, normal.x, normal.y, normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { c3.x, c3.y, c3.z, normal.x, normal.y, normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
    Renderer_PushVertex(renderer, { c4.x, c4.y, c4.z, normal.x, normal.y, normal.z, uv.x, uv.y, color.r, color.g, color.b, color.a });
}

void DrawCircle(Renderer* renderer, 
                glm::vec3 midpoint, glm::vec3 edgepoint, glm::vec3 normpoint, 
                int steps, 
                glm::u8vec4 surfaceColor,                                      // Fill Style
                glm::u8vec4 borderColor, float thickness, float dashLen, float gapLen) // Border Style
{
    if (steps < 3) steps = 36;
    if (!renderer) return;

    // 1. Calculate Basis Vectors
    glm::vec3 vectorU = edgepoint - midpoint;
    float radius = glm::length(vectorU);
    if (radius < 0.0001f) return;
    
    // Calculate Normal Direction
    glm::vec3 normDir = normpoint - midpoint;

    // Calculate Tangent (V) perpendicular to Radius (U) and Normal Direction
    glm::vec3 vectorV = glm::cross(normDir, vectorU);

    // Safety fallback
    if (glm::length(vectorV) < 0.001f) {
        glm::vec3 arbitrary = (glm::abs(vectorU.y) > 0.99f) ? glm::vec3(1,0,0) : glm::vec3(0,1,0);
        vectorV = glm::cross(arbitrary, vectorU);
    }

    // Normalize V and scale to radius
    vectorV = glm::normalize(vectorV) * radius;
    
    // Calculate the geometric normal for lighting
    glm::vec3 surfaceNormal = glm::normalize(glm::cross(vectorU, vectorV));

    // 2. Generate Points
    std::vector<glm::vec3> points;
    points.reserve(steps + 1);
    const float PI2 = glm::pi<float>() * 2.0f;

    for (int i = 0; i <= steps; i++) {
        float theta = (float)i / (float)steps * PI2;
        // Parametric Circle: Center + (RadiusVec * cos) + (TangentVec * sin)
        points.push_back(midpoint + (vectorU * cos(theta)) + (vectorV * sin(theta)));
    }

    // 3. Draw Surface (Single Face)
    if (1) {
        ImVec2 uv = ImGui::GetFontTexUvWhitePixel();
        
        // Triangle Fan
        for (int i = 0; i < steps; i++) {
            glm::vec3 p1 = points[i];
            glm::vec3 p2 = points[i + 1];

            // Single winding order (CCW)
            Renderer_PushVertex(renderer, { midpoint.x, midpoint.y, midpoint.z, surfaceNormal.x, surfaceNormal.y, surfaceNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
            Renderer_PushVertex(renderer, { p1.x, p1.y, p1.z, surfaceNormal.x, surfaceNormal.y, surfaceNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
            Renderer_PushVertex(renderer, { p2.x, p2.y, p2.z, surfaceNormal.x, surfaceNormal.y, surfaceNormal.z, uv.x, uv.y, surfaceColor.r, surfaceColor.g, surfaceColor.b, surfaceColor.a });
        }
    }

    // 4. Draw Border
    if (borderColor.a > 0 && thickness > 0.0f) {
        DrawSegment(renderer, points.data(), (int)points.size(), thickness, borderColor, dashLen, gapLen);
    }
}

void DrawSphere(Renderer* renderer, const Camera* cam,
                glm::vec3 center, glm::vec3 radiusPoint, 
                int sectorCount, int stackCount, glm::u8vec4 sphereColor, 
                glm::u8vec4 ringColor, float ringThickness, float ringDashLen, float ringGapLen) 
{
    if (!renderer) return;

    // 1. Calculate Radius
    float radius = glm::distance(center, radiusPoint);
    if (radius <= 0.0001f) return;

    const float PI = glm::pi<float>();
    ImVec2 uv = ImGui::GetFontTexUvWhitePixel();

    // --- PART A: Draw the 3D Sphere Mesh ---
    if (1) {
        for (int i = 0; i < stackCount; ++i) {
            float stackAngle1 = PI / 2.0f - (float)i / stackCount * PI;
            float stackAngle2 = PI / 2.0f - (float)(i + 1) / stackCount * PI;

            float xy1 = radius * cos(stackAngle1);
            float z1  = radius * sin(stackAngle1);
            float xy2 = radius * cos(stackAngle2);
            float z2  = radius * sin(stackAngle2);

            for (int j = 0; j < sectorCount; ++j) {
                float sectorAngle1 = (float)j / sectorCount * 2.0f * PI;
                float sectorAngle2 = (float)(j + 1) / sectorCount * 2.0f * PI;

                auto makeVertex = [&](float xy, float z, float theta) -> Vertex {
                    float x = xy * cos(theta);
                    float y = z; 
                    float z_coord = xy * sin(theta);
                    
                    glm::vec3 p = center + glm::vec3(x, y, z_coord);
                    glm::vec3 n = glm::normalize(p - center);
                    
                    return { p.x, p.y, p.z, n.x, n.y, n.z, uv.x, uv.y, sphereColor.r, sphereColor.g, sphereColor.b, sphereColor.a };
                };

                Vertex v1 = makeVertex(xy1, z1, sectorAngle1);
                Vertex v2 = makeVertex(xy2, z2, sectorAngle1);
                Vertex v3 = makeVertex(xy1, z1, sectorAngle2);
                Vertex v4 = makeVertex(xy2, z2, sectorAngle2);

                // FIX: Swapped vertex order to ensure Counter-Clockwise winding (Outward Facing)
                
                // Triangle 1: Top-Left -> Top-Right -> Bottom-Left
                if (i != 0) {
                    Renderer_PushVertex(renderer, v1);
                    Renderer_PushVertex(renderer, v3); // Swapped v2 and v3
                    Renderer_PushVertex(renderer, v2);
                }
                
                // Triangle 2: Top-Right -> Bottom-Right -> Bottom-Left
                if (i != stackCount - 1) {
                    Renderer_PushVertex(renderer, v3);
                    Renderer_PushVertex(renderer, v4); // Swapped v2 and v4 order
                    Renderer_PushVertex(renderer, v2);
                }
            }
        }
    }

    // --- PART B: Draw the Camera-Facing Ring ---
    if (cam && ringColor.a > 0 && ringThickness > 0.0f) {
        glm::vec3 viewDir = glm::normalize(cam->position - center);
        glm::vec3 arbitraryUp = (glm::abs(viewDir.y) > 0.99f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        glm::vec3 right = glm::normalize(glm::cross(arbitraryUp, viewDir));
        glm::vec3 up    = glm::normalize(glm::cross(viewDir, right));

        int ringSteps = (sectorCount < 48) ? 48 : sectorCount;
        std::vector<glm::vec3> ringPoints;
        ringPoints.reserve(ringSteps + 1);

        for (int i = 0; i <= ringSteps; i++) {
            float theta = (float)i / (float)ringSteps * 2.0f * PI;
            glm::vec3 offset = (right * cos(theta) + up * sin(theta)) * radius;
            ringPoints.push_back(center + offset);
        }

        DrawSegment(renderer, ringPoints.data(), (int)ringPoints.size(), ringThickness, ringColor, ringDashLen, ringGapLen);
    }
}

void DrawCylinder(Renderer* renderer, const Camera* cam,
                  glm::vec3 bottomCenter, glm::vec3 topCenter, glm::vec3 bottomEdge, 
                  int sectorCount, glm::u8vec4 cylinderColor,
                  glm::u8vec4 outlineColor, float outlineThickness, float dashLen, float gapLen) 
{
    if (!renderer || sectorCount < 3) return;

    // 1. Calculate Geometry
    glm::vec3 axisVec = topCenter - bottomCenter;
    float height = glm::length(axisVec);
    if (height <= 0.0001f) return;
    glm::vec3 axisDir = axisVec / height;

    glm::vec3 rawRadVec = bottomEdge - bottomCenter;
    glm::vec3 perpRadVec = rawRadVec - (glm::dot(rawRadVec, axisDir) * axisDir);
    float radius = glm::length(perpRadVec);
    if (radius <= 0.0001f) return;

    glm::vec3 basisX = glm::normalize(perpRadVec);
    glm::vec3 basisY = glm::normalize(glm::cross(axisDir, basisX));

    const float PI = glm::pi<float>();
    const float PI2 = PI * 2.0f;
    ImVec2 uv = ImGui::GetFontTexUvWhitePixel();

    // --- PART A: Draw 3D Cylinder Mesh ---
    if (1) {
        for (int i = 0; i < sectorCount; ++i) {
            float theta1 = (float)i / sectorCount * PI2;
            float theta2 = (float)(i + 1) / sectorCount * PI2;

            float c1 = cos(theta1), s1 = sin(theta1);
            float c2 = cos(theta2), s2 = sin(theta2);

            glm::vec3 p1_off = (basisX * c1 + basisY * s1) * radius;
            glm::vec3 p2_off = (basisX * c2 + basisY * s2) * radius;

            glm::vec3 b1 = bottomCenter + p1_off;
            glm::vec3 b2 = bottomCenter + p2_off;
            glm::vec3 t1 = topCenter + p1_off;
            glm::vec3 t2 = topCenter + p2_off;

            glm::vec3 n1 = glm::normalize(p1_off);
            glm::vec3 n2 = glm::normalize(p2_off);
            glm::vec3 topN = axisDir;
            glm::vec3 botN = -axisDir;

            // Side Wall
            Renderer_PushVertex(renderer, { b1.x, b1.y, b1.z, n1.x, n1.y, n1.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { b2.x, b2.y, b2.z, n2.x, n2.y, n2.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { t1.x, t1.y, t1.z, n1.x, n1.y, n1.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });

            Renderer_PushVertex(renderer, { t1.x, t1.y, t1.z, n1.x, n1.y, n1.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { b2.x, b2.y, b2.z, n2.x, n2.y, n2.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { t2.x, t2.y, t2.z, n2.x, n2.y, n2.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });

            // Bottom Cap
            Renderer_PushVertex(renderer, { bottomCenter.x, bottomCenter.y, bottomCenter.z, botN.x, botN.y, botN.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { b2.x, b2.y, b2.z, botN.x, botN.y, botN.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { b1.x, b1.y, b1.z, botN.x, botN.y, botN.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });

            // Top Cap
            Renderer_PushVertex(renderer, { topCenter.x, topCenter.y, topCenter.z, topN.x, topN.y, topN.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { t1.x, t1.y, t1.z, topN.x, topN.y, topN.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
            Renderer_PushVertex(renderer, { t2.x, t2.y, t2.z, topN.x, topN.y, topN.z, uv.x, uv.y, cylinderColor.r, cylinderColor.g, cylinderColor.b, cylinderColor.a });
        }
    }

    // --- PART B: Draw Outline ---
    if (cam && outlineColor.a > 0 && outlineThickness > 0.0f) {
        // 1. Rings
        int ringSteps = (sectorCount < 48) ? 48 : sectorCount;
        std::vector<glm::vec3> botPoints(ringSteps + 1);
        std::vector<glm::vec3> topPoints(ringSteps + 1);

        for (int i = 0; i <= ringSteps; i++) {
            float theta = (float)i / ringSteps * PI2;
            glm::vec3 offset = (basisX * cos(theta) + basisY * sin(theta)) * radius;
            botPoints[i] = bottomCenter + offset;
            topPoints[i] = topCenter + offset;
        }
        DrawSegment(renderer, botPoints.data(), (int)botPoints.size(), outlineThickness, outlineColor, dashLen, gapLen);
        DrawSegment(renderer, topPoints.data(), (int)topPoints.size(), outlineThickness, outlineColor, dashLen, gapLen);

        // 2. Silhouette Lines
        glm::vec3 midPoint = (bottomCenter + topCenter) * 0.5f;
        glm::vec3 camToMid = cam->position - midPoint;
        
        // Project onto plane perpendicular to axis
        glm::vec3 projCamVec = camToMid - (glm::dot(camToMid, axisDir) * axisDir);
        
        // Tangent direction
        glm::vec3 tangentDir = glm::normalize(glm::cross(projCamVec, axisDir));
        glm::vec3 rimOffset = tangentDir * radius;

        glm::vec3 line1[2] = { bottomCenter + rimOffset, topCenter + rimOffset };
        glm::vec3 line2[2] = { bottomCenter - rimOffset, topCenter - rimOffset };

        DrawSegment(renderer, line1, 2, outlineThickness, outlineColor, dashLen, gapLen);
        DrawSegment(renderer, line2, 2, outlineThickness, outlineColor, dashLen, gapLen);
    }
}

void DrawCone(Renderer* renderer, const Camera* cam,
              glm::vec3 bottomCenter, glm::vec3 topCenter, glm::vec3 bottomEdge, 
              int sectorCount, glm::u8vec4 coneColor,
              glm::u8vec4 outlineColor, float outlineThickness, float dashLen, float gapLen) 
{
    if (!renderer || sectorCount < 3) return;

    // 1. Geometry
    glm::vec3 axisVec = topCenter - bottomCenter;
    float height = glm::length(axisVec);
    if (height <= 0.0001f) return;
    glm::vec3 axisDir = axisVec / height;

    glm::vec3 rawRadVec = bottomEdge - bottomCenter;
    glm::vec3 perpRadVec = rawRadVec - (glm::dot(rawRadVec, axisDir) * axisDir);
    float radius = glm::length(perpRadVec);
    if (radius <= 0.0001f) return;

    glm::vec3 basisX = glm::normalize(perpRadVec);
    glm::vec3 basisY = glm::normalize(glm::cross(axisDir, basisX));

    float hypotenuse = sqrt(height * height + radius * radius);
    float nY = radius / hypotenuse; 
    float nR = height / hypotenuse; 

    const float PI = glm::pi<float>();
    const float PI2 = PI * 2.0f;
    ImVec2 uv = ImGui::GetFontTexUvWhitePixel();

    // --- PART A: Draw 3D Cone Mesh ---
    if (1) {
        for (int i = 0; i < sectorCount; ++i) {
            float theta1 = (float)i / sectorCount * PI2;
            float theta2 = (float)(i + 1) / sectorCount * PI2;

            float c1 = cos(theta1), s1 = sin(theta1);
            float c2 = cos(theta2), s2 = sin(theta2);

            glm::vec3 p1_off = (basisX * c1 + basisY * s1) * radius;
            glm::vec3 p2_off = (basisX * c2 + basisY * s2) * radius;

            glm::vec3 b1 = bottomCenter + p1_off;
            glm::vec3 b2 = bottomCenter + p2_off;

            glm::vec3 r1 = glm::normalize(p1_off);
            glm::vec3 r2 = glm::normalize(p2_off);
            glm::vec3 sideN1 = glm::normalize(r1 * nR + axisDir * nY);
            glm::vec3 sideN2 = glm::normalize(r2 * nR + axisDir * nY);
            glm::vec3 botN = -axisDir;

            // Side Triangle
            Renderer_PushVertex(renderer, { topCenter.x, topCenter.y, topCenter.z, sideN1.x, sideN1.y, sideN1.z, uv.x, uv.y, coneColor.r, coneColor.g, coneColor.b, coneColor.a });
            Renderer_PushVertex(renderer, { b1.x, b1.y, b1.z, sideN1.x, sideN1.y, sideN1.z, uv.x, uv.y, coneColor.r, coneColor.g, coneColor.b, coneColor.a });
            Renderer_PushVertex(renderer, { b2.x, b2.y, b2.z, sideN2.x, sideN2.y, sideN2.z, uv.x, uv.y, coneColor.r, coneColor.g, coneColor.b, coneColor.a });

            // Base Cap Triangle
            Renderer_PushVertex(renderer, { bottomCenter.x, bottomCenter.y, bottomCenter.z, botN.x, botN.y, botN.z, uv.x, uv.y, coneColor.r, coneColor.g, coneColor.b, coneColor.a });
            Renderer_PushVertex(renderer, { b2.x, b2.y, b2.z, botN.x, botN.y, botN.z, uv.x, uv.y, coneColor.r, coneColor.g, coneColor.b, coneColor.a });
            Renderer_PushVertex(renderer, { b1.x, b1.y, b1.z, botN.x, botN.y, botN.z, uv.x, uv.y, coneColor.r, coneColor.g, coneColor.b, coneColor.a });
        }
    }

    // --- PART B: Draw Outline ---
    if (cam && outlineColor.a > 0 && outlineThickness > 0.0f) {
        // 1. Base Ring
        int ringSteps = (sectorCount < 48) ? 48 : sectorCount;
        std::vector<glm::vec3> botPoints(ringSteps + 1);

        for (int i = 0; i <= ringSteps; i++) {
            float theta = (float)i / ringSteps * PI2;
            glm::vec3 offset = (basisX * cos(theta) + basisY * sin(theta)) * radius;
            botPoints[i] = bottomCenter + offset;
        }
        DrawSegment(renderer, botPoints.data(), (int)botPoints.size(), outlineThickness, outlineColor, dashLen, gapLen);
        
        // 2. Silhouette Lines
        glm::vec3 camToBottom = cam->position - bottomCenter;
        
        // Project onto plane perpendicular to axis
        glm::vec3 projCamVec = camToBottom - (glm::dot(camToBottom, axisDir) * axisDir);
        
        glm::vec3 tangentDir = glm::normalize(glm::cross(projCamVec, axisDir));
        glm::vec3 rimOffset = tangentDir * radius;

        glm::vec3 line1[2] = { bottomCenter + rimOffset, topCenter };
        glm::vec3 line2[2] = { bottomCenter - rimOffset, topCenter };

        DrawSegment(renderer, line1, 2, outlineThickness, outlineColor, dashLen, gapLen);
        DrawSegment(renderer, line2, 2, outlineThickness, outlineColor, dashLen, gapLen);
    }
}

// -----------------------------------------------------------------------------
// MATH & LOGIC
// -----------------------------------------------------------------------------

void RecalculatePoint(DrawEntities& entities, int idx) {
    if (idx < 0 || idx >= (int)entities.points.size()) return;
    Point& p = entities.points[idx];

    auto ParentPos = [&](int pIdx) -> glm::vec3 {
        if (pIdx >= 0 && pIdx < (int)entities.points.size()) return entities.points[pIdx].pos;
        return glm::vec3(0.0f);
    };

    switch (p.op) {
        case OP_FREE: break;
        case OP_DILATE: {
            if (p.parents.size() >= 2) {
                glm::vec3 target = ParentPos(p.parents[0]);
                glm::vec3 center = ParentPos(p.parents[1]);
                p.pos = center + (target - center) * p.param;
            }
            break;
        }
        case OP_TRANSLATE: {
            if (p.parents.size() >= 3) {
                glm::vec3 target = ParentPos(p.parents[0]);
                glm::vec3 start = ParentPos(p.parents[1]);
                glm::vec3 end   = ParentPos(p.parents[2]);
                p.pos = target + (end - start);
            }
            break;
        }
        case OP_ROTATE_LINE: {
            if (p.parents.size() >= 3) {
                glm::vec3 target = ParentPos(p.parents[0]);
                glm::vec3 axisA  = ParentPos(p.parents[1]);
                glm::vec3 axisB  = ParentPos(p.parents[2]);
                glm::vec3 axisDir = axisB - axisA;
                if (glm::length(axisDir) > 0.0001f) {
                    axisDir = glm::normalize(axisDir);
                    float rads = glm::radians(p.param);
                    p.pos = axisA + glm::rotate(target - axisA, rads, axisDir);
                }
            }
            break;
        }
        case OP_PROJECT_LINE: {
            if (p.parents.size() >= 3) {
                glm::vec3 target = ParentPos(p.parents[0]);
                glm::vec3 lA = ParentPos(p.parents[1]);
                glm::vec3 lB = ParentPos(p.parents[2]);
                glm::vec3 lineDir = lB - lA;
                float len2 = glm::dot(lineDir, lineDir);
                if (len2 > 0.0001f) {
                    float t = glm::dot(target - lA, lineDir) / len2;
                    p.pos = lA + lineDir * t;
                }
            }
            break;
        }
        case OP_REFLECT_PLANE: {
            if (p.parents.size() >= 4) {
                glm::vec3 target = ParentPos(p.parents[0]);
                glm::vec3 p1 = ParentPos(p.parents[1]);
                glm::vec3 p2 = ParentPos(p.parents[2]);
                glm::vec3 p3 = ParentPos(p.parents[3]);
                glm::vec3 normal = glm::cross(p2 - p1, p3 - p1);
                if (glm::length(normal) > 0.0001f) {
                    normal = glm::normalize(normal);
                    float dist = glm::dot(target - p1, normal);
                    p.pos = target - (2.0f * dist * normal);
                }
            }
            break;
        }
        case OP_PERP_TO_PLANE: {
            if (p.parents.size() >= 4) {
                glm::vec3 target = ParentPos(p.parents[0]);
                glm::vec3 p1 = ParentPos(p.parents[1]);
                glm::vec3 p2 = ParentPos(p.parents[2]);
                glm::vec3 p3 = ParentPos(p.parents[3]);
                glm::vec3 normal = glm::cross(p2 - p1, p3 - p1);
                if (glm::length(normal) > 0.0001f) {
                    normal = glm::normalize(normal);
                    p.pos = target + normal * p.param;
                }
            }
            break;
        }
        case OP_ANGLE_DIV: {
            if (p.parents.size() >= 3) {
                glm::vec3 start = ParentPos(p.parents[0]);
                glm::vec3 vert = ParentPos(p.parents[1]);
                glm::vec3 end = ParentPos(p.parents[2]);
                glm::vec3 u = start - vert;
                glm::vec3 v = end - vert;
                if (glm::length(u) > 0.0001f && glm::length(v) > 0.0001f) {
                    u = glm::normalize(u); v = glm::normalize(v);
                    glm::vec3 axis = glm::cross(u, v);
                    if (glm::length(axis) < 0.0001f) axis = glm::vec3(0,1,0);
                    else axis = glm::normalize(axis);
                    float totalAngle = glm::angle(u, v);
                    float dist = glm::distance(start, vert);
                    p.pos = vert + glm::rotate(u, totalAngle * p.param, axis) * dist;
                }
            }
            break;
        }
        case OP_INTERSECT_LINES: {
            if (p.parents.size() >= 4) {
                glm::vec3 p1 = ParentPos(p.parents[0]);
                glm::vec3 p2 = ParentPos(p.parents[1]);
                glm::vec3 p3 = ParentPos(p.parents[2]);
                glm::vec3 p4 = ParentPos(p.parents[3]);
                glm::vec3 d1 = p2 - p1;
                glm::vec3 d2 = p4 - p3;
                glm::vec3 r  = p1 - p3;
                float a = glm::dot(d1, d1);
                float b = glm::dot(d1, d2);
                float c = glm::dot(d1, r);
                float e = glm::dot(d2, d2);
                float d = glm::dot(d2, r);
                float denom = a * e - b * b;
                if (denom < 0.00001f) p.pos = p1; 
                else {
                    float s = (b * d - c * e) / denom;
                    p.pos = p1 + d1 * s;
                }
            }
            break;
        }
    }
}

void PropagateUpdates(DrawEntities& entities, int startNodeIdx) {
    if (startNodeIdx < 0 || startNodeIdx >= (int)entities.points.size()) return;
    RecalculatePoint(entities, startNodeIdx);
    for (int childIdx : entities.points[startNodeIdx].children) {
        PropagateUpdates(entities, childIdx);
    }
}

void SetPointParent(DrawEntities& entities, int childIdx, int parentSlot, int newParentIdx) {
    if (childIdx < 0 || childIdx >= (int)entities.points.size()) return;
    Point& child = entities.points[childIdx];
    if (parentSlot < 0 || parentSlot >= (int)child.parents.size()) return;

    int oldParentIdx = child.parents[parentSlot];
    if (oldParentIdx >= 0 && oldParentIdx < (int)entities.points.size()) {
        auto& siblings = entities.points[oldParentIdx].children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), childIdx), siblings.end());
    }

    child.parents[parentSlot] = newParentIdx;

    if (newParentIdx >= 0 && newParentIdx < (int)entities.points.size()) {
        entities.points[newParentIdx].children.push_back(childIdx);
    }
    PropagateUpdates(entities, childIdx);
}

int CreatePoint(DrawEntities& entities, Operation op, const std::vector<int>& parents, float param = 0.0f) {
    Point p;
    p.name = "C" + std::to_string(entities.points.size());
    p.op = op;
    p.parents = parents;
    p.param = param;
    
    int newIdx = (int)entities.points.size();
    for (int parentId : parents) {
        if (parentId >= 0 && parentId < (int)entities.points.size()) {
            entities.points[parentId].children.push_back(newIdx);
        }
    }
    entities.points.push_back(p);
    RecalculatePoint(entities, newIdx);
    return newIdx;
}

static bool PointSelector(const char* label, int* currentIdx, const std::vector<Point>& points) {
    if (points.empty()) { ImGui::Text("%s: No Pts", label); return false; }
    if (*currentIdx < 0) *currentIdx = 0;
    if (*currentIdx >= (int)points.size()) *currentIdx = (int)points.size() - 1;
    
    std::string preview = std::to_string(*currentIdx) + ": " + points[*currentIdx].name;
    bool changed = false;
    
    ImGui::PushID(label); 
    if (ImGui::BeginCombo(label, preview.c_str())) {
        for (int i = 0; i < (int)points.size(); i++) {
            const bool is_selected = (*currentIdx == i);
            std::string item_label = std::to_string(i) + ": " + points[i].name;
            if (ImGui::Selectable(item_label.c_str(), is_selected)) {
                *currentIdx = i; changed = true;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
    return changed;
}

static void EditColor(const char* label, glm::u8vec4& col) {
    float c[4] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f };
    if (ImGui::ColorEdit4(label, c)) {
        col = { (unsigned char)(c[0]*255), (unsigned char)(c[1]*255), (unsigned char)(c[2]*255), (unsigned char)(c[3]*255) };
    }
}

static void EditLineStyle(float& thick, float& dash, float& gap) {
    ImGui::DragFloat("Thick", &thick, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Dash", &dash, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Gap", &gap, 0.01f, 0.0f, 10.0f);
}

static void DeletePoint(DrawEntities& entities, int indexToDelete) {
    if (indexToDelete < 0 || indexToDelete >= (int)entities.points.size()) return;

    if (!entities.points[indexToDelete].children.empty()) {
        for(int child : entities.points[indexToDelete].children) {
            auto& parents = entities.points[child].parents;
            parents.erase(std::remove(parents.begin(), parents.end(), indexToDelete), parents.end());
            entities.points[child].op = OP_FREE; 
        }
    }
    for(int parent : entities.points[indexToDelete].parents) {
        auto& siblings = entities.points[parent].children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), indexToDelete), siblings.end());
    }

    entities.points.erase(entities.points.begin() + indexToDelete);
    auto FixIdx = [&](int& idx) { if (idx == indexToDelete) idx = 0; else if (idx > indexToDelete) idx--; };

    for (auto& p : entities.points) { for(int& pid : p.parents) FixIdx(pid); for(int& cid : p.children) FixIdx(cid); }
    for (auto& l : entities.lines) { FixIdx(l.point1); FixIdx(l.point2); }
    for (auto& r : entities.rays)  { FixIdx(r.point1); FixIdx(r.point2); }
    for (auto& s : entities.segments) { FixIdx(s.point1); FixIdx(s.point2); }
    for (auto& surf : entities.surfaces) { for (int& pid : surf.pointIndices) FixIdx(pid); }
    for (auto& p : entities.planes) { FixIdx(p.point1); FixIdx(p.point2); FixIdx(p.point3); }
    for (auto& c : entities.circles) { FixIdx(c.midpoint); FixIdx(c.edgepoint); FixIdx(c.normpoint); }
    for (auto& s : entities.spheres) { FixIdx(s.midpoint); FixIdx(s.edgepoint); }
    for (auto& c : entities.cylinders) { FixIdx(c.midpoint); FixIdx(c.edgepoint); FixIdx(c.toppoint); }
    for (auto& c : entities.cones) { FixIdx(c.midpoint); FixIdx(c.edgepoint); FixIdx(c.toppoint); }
    for (auto& a : entities.angles) { FixIdx(a.p1); FixIdx(a.vertex); FixIdx(a.p2); }
}

// =============================================================================
//  SERIALIZATION HELPERS
// =============================================================================

static std::ostream& operator<<(std::ostream& os, const glm::u8vec4& c) {
    os << (int)c.r << " " << (int)c.g << " " << (int)c.b << " " << (int)c.a;
    return os;
}

static std::istream& operator>>(std::istream& is, glm::u8vec4& c) {
    int r, g, b, a;
    is >> r >> g >> b >> a;
    c = {(glm::uint8)r, (glm::uint8)g, (glm::uint8)b, (glm::uint8)a};
    return is;
}

static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    os << v.x << " " << v.y << " " << v.z;
    return os;
}

static std::istream& operator>>(std::istream& is, glm::vec3& v) {
    is >> v.x >> v.y >> v.z;
    return is;
}

void SaveScene(const DrawEntities& entities, const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) return;

    out << "GLOBAL " << entities.globalTextSize << " " << entities.globalTextOffset << "\n";

    // 1. Points
    out << "POINTS " << entities.points.size() << "\n";
    for (const auto& p : entities.points) {
        // Format: Name Op Param VisualRadius Fixed ShowLabel
        // To simplify name saving, we replace spaces with underscores or assume single word. 
        // For this example, we assume name is safe or quote it.
        std::string safeName = p.name.empty() ? "Unamed" : p.name; 
        
        out << safeName << " " << (int)p.op << " " << p.param << " " 
            << p.visualRadius << " " << p.fixed << " " << p.showLabel << "\n";
        
        // Colors and Pos
        out << p.pos << " " << p.color << " " << p.textColor << "\n";

        // Parents
        out << p.parents.size();
        for (int pid : p.parents) out << " " << pid;
        out << "\n";
    }

    // 2. Lines
    out << "LINES " << entities.lines.size() << "\n";
    for (const auto& l : entities.lines) {
        out << l.point1 << " " << l.point2 << " " << l.thickness << " " << l.dashLen << " " << l.gapLen << " " << l.color << "\n";
    }

    // 3. Segments
    out << "SEGMENTS " << entities.segments.size() << "\n";
    for (const auto& s : entities.segments) {
        out << s.point1 << " " << s.point2 << " " << s.thickness << " " << s.dashLen << " " << s.gapLen << " " << s.color << "\n";
    }

    // 4. Rays
    out << "RAYS " << entities.rays.size() << "\n";
    for (const auto& r : entities.rays) {
        out << r.point1 << " " << r.point2 << " " << r.thickness << " " << r.dashLen << " " << r.gapLen << " " << r.color << "\n";
    }

    // 5. Angles
    out << "ANGLES " << entities.angles.size() << "\n";
    for (const auto& a : entities.angles) {
        out << a.p1 << " " << a.vertex << " " << a.p2 << " " << a.radius << " " << a.thickness << " " << a.color << "\n";
    }

    // 6. Planes
    out << "PLANES " << entities.planes.size() << "\n";
    for (const auto& pl : entities.planes) {
        out << pl.point1 << " " << pl.point2 << " " << pl.point3 << " " << pl.color << "\n";
    }

    // 7. Circles
    out << "CIRCLES " << entities.circles.size() << "\n";
    for (const auto& c : entities.circles) {
        out << c.midpoint << " " << c.edgepoint << " " << c.normpoint << " " << c.steps << " " 
            << c.thickness << " " << c.dashLen << " " << c.gapLen << " " 
            << c.surfaceColor << " " << c.borderColor << "\n";
    }

    // 8. Surfaces
    out << "SURFACES " << entities.surfaces.size() << "\n";
    for (const auto& s : entities.surfaces) {
        out << s.surfaceColor << " " << s.borderColor << " " 
            << s.borderThickness << " " << s.dashLen << " " << s.gapLen << "\n";
        out << s.pointIndices.size();
        for (int idx : s.pointIndices) out << " " << idx;
        out << "\n";
    }

    // 9. Spheres
    out << "SPHERES " << entities.spheres.size() << "\n";
    for(const auto& s : entities.spheres) {
        out << s.midpoint << " " << s.edgepoint << " " << s.sectorCount << " " << s.stackCount << " "
            << s.ringThickness << " " << s.dashLen << " " << s.gapLen << " "
            << s.sphereColor << " " << s.ringColor << "\n";
    }

    // 10. Cylinders
    out << "CYLINDERS " << entities.cylinders.size() << "\n";
    for(const auto& c : entities.cylinders) {
        out << c.midpoint << " " << c.edgepoint << " " << c.toppoint << " " << c.sectorCount << " "
            << c.outlineThickness << " " << c.dashLen << " " << c.gapLen << " "
            << c.cylinderColor << " " << c.outlineColor << "\n";
    }

    // 11. Cones
    out << "CONES " << entities.cones.size() << "\n";
    for(const auto& c : entities.cones) {
        out << c.midpoint << " " << c.edgepoint << " " << c.toppoint << " " << c.sectorCount << " "
            << c.outlineThickness << " " << c.dashLen << " " << c.gapLen << " "
            << c.coneColor << " " << c.outlineColor << "\n";
    }

    out.close();
}

void LoadScene(DrawEntities& entities, const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return;
    }

    // Clear existing
    entities = DrawEntities(); 

    std::string header;
    size_t count;
    
    // Globals
    in >> header; 
    if (header == "GLOBAL") {
        in >> entities.globalTextSize >> entities.globalTextOffset;
    }

    // Helper to skip potential headers if order changes or simple parsing
    auto CheckHeader = [&](const std::string& expected) {
        std::string h; 
        // Peek or read until we find header? Simple approach: assume strict order for now
        in >> h; 
        if (h != expected) std::cerr << "Warning: Expected " << expected << " got " << h << std::endl;
        in >> count;
    };
    
    // 1. Points
    CheckHeader("POINTS");
    entities.points.resize(count);
    for (size_t i = 0; i < count; i++) {
        Point& p = entities.points[i];
        int opInt;
        in >> p.name >> opInt >> p.param >> p.visualRadius >> p.fixed >> p.showLabel;
        p.op = (Operation)opInt;
        in >> p.pos >> p.color >> p.textColor;

        
        size_t pCount;
        in >> pCount;
        p.parents.resize(pCount);

        for(size_t k=0; k<pCount; k++) in >> p.parents[k];

    }

    // REBUILD GRAPH: Re-populate 'children' arrays based on loaded 'parents'
    for (int i = 0; i < (int)entities.points.size(); i++) {
        for (int parentIdx : entities.points[i].parents) {
            if (parentIdx >= 0 && parentIdx < (int)entities.points.size()) {
                entities.points[parentIdx].children.push_back(i);
            }
        }
    }
    // Recalculate positions to ensure consistency
    if (!entities.points.empty()) PropagateUpdates(entities, 0);

    // 2. Lines
    CheckHeader("LINES");
    entities.lines.resize(count);
    for (size_t i = 0; i < count; i++) {
        Line& l = entities.lines[i];
        in >> l.point1 >> l.point2 >> l.thickness >> l.dashLen >> l.gapLen >> l.color;
    }

    // 3. Segments
    CheckHeader("SEGMENTS");
    entities.segments.resize(count);
    for (size_t i = 0; i < count; i++) {
        Segment& s = entities.segments[i];
        in >> s.point1 >> s.point2 >> s.thickness >> s.dashLen >> s.gapLen >> s.color;
    }

    // 4. Rays
    CheckHeader("RAYS");
    entities.rays.resize(count);
    for (size_t i = 0; i < count; i++) {
        Ray& r = entities.rays[i];
        in >> r.point1 >> r.point2 >> r.thickness >> r.dashLen >> r.gapLen >> r.color;
    }

    // 5. Angles
    CheckHeader("ANGLES");
    entities.angles.resize(count);
    for (size_t i = 0; i < count; i++) {
        AngleMeas& a = entities.angles[i];
        in >> a.p1 >> a.vertex >> a.p2 >> a.radius >> a.thickness >> a.color;
    }

    // 6. Planes
    CheckHeader("PLANES");
    entities.planes.resize(count);
    for (size_t i = 0; i < count; i++) {
        Plane& pl = entities.planes[i];
        in >> pl.point1 >> pl.point2 >> pl.point3 >> pl.color;
    }

    // 7. Circles
    CheckHeader("CIRCLES");
    entities.circles.resize(count);
    for (size_t i = 0; i < count; i++) {
        Circle& c = entities.circles[i];
        in >> c.midpoint >> c.edgepoint >> c.normpoint >> c.steps 
           >> c.thickness >> c.dashLen >> c.gapLen 
           >> c.surfaceColor >> c.borderColor;
    }

    // 8. Surfaces
    CheckHeader("SURFACES");
    entities.surfaces.resize(count);
    for (size_t i = 0; i < count; i++) {
        Surface& s = entities.surfaces[i];
        in >> s.surfaceColor >> s.borderColor >> s.borderThickness >> s.dashLen >> s.gapLen;
        size_t idxCount;
        in >> idxCount;
        s.pointIndices.resize(idxCount);
        for(size_t k=0; k<idxCount; k++) in >> s.pointIndices[k];
    }

    // 9. Spheres
    CheckHeader("SPHERES");
    entities.spheres.resize(count);
    for(size_t i=0; i<count; i++) {
        Sphere& s = entities.spheres[i];
        in >> s.midpoint >> s.edgepoint >> s.sectorCount >> s.stackCount
           >> s.ringThickness >> s.dashLen >> s.gapLen
           >> s.sphereColor >> s.ringColor;
    }

    // 10. Cylinders
    CheckHeader("CYLINDERS");
    entities.cylinders.resize(count);
    for(size_t i=0; i<count; i++) {
        Cylinder& c = entities.cylinders[i];
        in >> c.midpoint >> c.edgepoint >> c.toppoint >> c.sectorCount
           >> c.outlineThickness >> c.dashLen >> c.gapLen
           >> c.cylinderColor >> c.outlineColor;
    }

    // 11. Cones
    CheckHeader("CONES");
    entities.cones.resize(count);
    for(size_t i=0; i<count; i++) {
        Cone& c = entities.cones[i];
        in >> c.midpoint >> c.edgepoint >> c.toppoint >> c.sectorCount
           >> c.outlineThickness >> c.dashLen >> c.gapLen
           >> c.coneColor >> c.outlineColor;
    }

}

// -----------------------------------------------------------------------------
// EDITOR UI
// -----------------------------------------------------------------------------

void DrawGeometryEditor(DrawEntities& entities) {
    ImGui::Begin("Geometry Drawer");

    // Local state for the File IO and Popups
    static char filenameBuf[128] = "scene.txt";
    static bool openConfirmPopup = false;
    static int pendingAction = 0; // 0 = None, 1 = Load, 2 = Clear

    // ==========================================
    // 1. FILE OPERATIONS
    // ==========================================
    if (ImGui::CollapsingHeader("File Operations", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Filename", filenameBuf, sizeof(filenameBuf));
        
        // SAVE
        if (ImGui::Button("Save Scene")) {
            SaveScene(entities, filenameBuf);
        }
        
        ImGui::SameLine();
        
        // LOAD
        if (ImGui::Button("Load Scene")) {
            if (entities.points.empty()) {
                LoadScene(entities, filenameBuf);
            } else {
                pendingAction = 1; // Mark as Load
                openConfirmPopup = true;
            }
        }

        ImGui::SameLine();

        // CLEAR
        if (ImGui::Button("Clear Scene")) {
            if (!entities.points.empty()) {
                pendingAction = 2; // Mark as Clear
                openConfirmPopup = true;
            }
        }
    }

    // --- POPUP MODAL LOGIC ---
    if (openConfirmPopup) {
        ImGui::OpenPopup("Confirm Action");
        openConfirmPopup = false;
    }

    // Center the popup
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Confirm Action", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        const char* actionName = (pendingAction == 1) ? "LOAD" : "CLEAR";
        ImGui::Text("This will %s the scene.\nUnsaved changes will be lost.\n\nAre you sure?", actionName);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (pendingAction == 1) {
                LoadScene(entities, filenameBuf);
            } else if (pendingAction == 2) {
                entities = DrawEntities(); // Reset to empty
            }
            pendingAction = 0;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            pendingAction = 0;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // ==========================================
    // 2. GLOBAL SETTINGS
    // ==========================================
    if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Text Size", &entities.globalTextSize, 0.1f, 60.0f);
        ImGui::DragFloat("Text Offset Y", &entities.globalTextOffset, 0.01f);
    }

    // ==========================================
    // 3. POINTS LIST & CONSTRUCTION
    // ==========================================
    if (ImGui::CollapsingHeader("Points List & Tools", ImGuiTreeNodeFlags_DefaultOpen)) {
        // --- CREATION BUTTONS ---
        ImGui::Text("Tools:");
        if (ImGui::Button("Free Point")) { Point p; p.name = "P" + std::to_string(entities.points.size()); entities.points.push_back(p); } ImGui::SameLine();
        if (ImGui::Button("Dilate")) CreatePoint(entities, OP_DILATE, {0, 0}, 2.0f); ImGui::SameLine();
        if (ImGui::Button("Translate")) CreatePoint(entities, OP_TRANSLATE, {0, 0, 0}); ImGui::SameLine();
        if (ImGui::Button("Rotate")) CreatePoint(entities, OP_ROTATE_LINE, {0, 0, 0}, 90.0f);
        
        if (ImGui::Button("Intersect")) CreatePoint(entities, OP_INTERSECT_LINES, {0, 0, 0, 0}); ImGui::SameLine();
        if (ImGui::Button("Project")) CreatePoint(entities, OP_PROJECT_LINE, {0, 0, 0}); ImGui::SameLine();
        if (ImGui::Button("Reflect Pl")) CreatePoint(entities, OP_REFLECT_PLANE, {0, 0, 0, 0});
        
        if (ImGui::Button("Perp Plane")) CreatePoint(entities, OP_PERP_TO_PLANE, {0, 0, 0, 0}, 1.0f); ImGui::SameLine();
        if (ImGui::Button("Angle Div")) CreatePoint(entities, OP_ANGLE_DIV, {0, 0, 0}, 0.5f);

        ImGui::Separator();
        
        // --- POINTS LIST ---
        int point_to_delete = -1;
        for (int i = 0; i < (int)entities.points.size(); i++) {
            ImGui::PushID(i);
            Point& p = entities.points[i];
            
            // Delete Button
            if (ImGui::Button("X")) { point_to_delete = i; } ImGui::SameLine();
            ImGui::Text("ID %d:", i); ImGui::SameLine();
            
            // Name Input
            char buf[64]; snprintf(buf, 64, "%s", p.name.c_str());
            ImGui::SetNextItemWidth(60);
            if (ImGui::InputText("##Name", buf, 64)) p.name = buf;
            ImGui::SameLine();

            // Logic based on Operation Type
            if (p.op == OP_FREE) {
                ImGui::SetNextItemWidth(150);
                // If user drags float, we need to propagate updates to children immediately
                if (ImGui::DragFloat3("##Pos", &p.pos.x, 0.1f)) {
                    PropagateUpdates(entities, i);
                }
            } 
            else {
                ImGui::TextColored(ImVec4(0,1,1,1), "[Const]");
                
                // Macro to make selecting parents easier
                #define UPDATE_PARENT(slot, label) { \
                    int tmp = p.parents[slot]; \
                    if (PointSelector(label, &tmp, entities.points)) { \
                        SetPointParent(entities, i, slot, tmp); \
                    } \
                }

                if (p.op == OP_DILATE) {
                    if (p.parents.size() < 2) p.parents.resize(2, 0);
                    UPDATE_PARENT(0, "Target"); UPDATE_PARENT(1, "Center");
                    ImGui::SetNextItemWidth(80); 
                    if (ImGui::DragFloat("Scale", &p.param, 0.01f)) PropagateUpdates(entities, i);
                }
                else if (p.op == OP_TRANSLATE) {
                    if (p.parents.size() < 3) p.parents.resize(3, 0);
                    UPDATE_PARENT(0, "Target"); UPDATE_PARENT(1, "Start"); UPDATE_PARENT(2, "End");
                }
                else if (p.op == OP_ROTATE_LINE) {
                    if (p.parents.size() < 3) p.parents.resize(3, 0);
                    UPDATE_PARENT(0, "Target"); UPDATE_PARENT(1, "Axis A"); UPDATE_PARENT(2, "Axis B");
                    if (ImGui::DragFloat("Deg", &p.param, 1.0f)) PropagateUpdates(entities, i);
                }
                else if (p.op == OP_INTERSECT_LINES) {
                    if (p.parents.size() < 4) p.parents.resize(4, 0);
                    UPDATE_PARENT(0, "L1 A"); UPDATE_PARENT(1, "L1 B"); UPDATE_PARENT(2, "L2 A"); UPDATE_PARENT(3, "L2 B");
                }
                else if (p.op == OP_PROJECT_LINE) {
                    if (p.parents.size() < 3) p.parents.resize(3, 0);
                    UPDATE_PARENT(0, "Target"); UPDATE_PARENT(1, "Line A"); UPDATE_PARENT(2, "Line B");
                }
                else if (p.op == OP_REFLECT_PLANE) {
                    if (p.parents.size() < 4) p.parents.resize(4, 0);
                    UPDATE_PARENT(0, "Target"); UPDATE_PARENT(1, "Pln 1"); UPDATE_PARENT(2, "Pln 2"); UPDATE_PARENT(3, "Pln 3");
                }
                else if (p.op == OP_PERP_TO_PLANE) {
                    if (p.parents.size() < 4) p.parents.resize(4, 0);
                    UPDATE_PARENT(0, "Target"); UPDATE_PARENT(1, "Pln 1"); UPDATE_PARENT(2, "Pln 2"); UPDATE_PARENT(3, "Pln 3");
                    if (ImGui::DragFloat("Dist", &p.param, 0.1f)) PropagateUpdates(entities, i);
                }
                else if (p.op == OP_ANGLE_DIV) {
                    if (p.parents.size() < 3) p.parents.resize(3, 0);
                    UPDATE_PARENT(0, "Start"); UPDATE_PARENT(1, "Vertex"); UPDATE_PARENT(2, "End");
                    if (ImGui::DragFloat("Ratio", &p.param, 0.01f)) PropagateUpdates(entities, i);
                }
            }
            
            // Point Style Expander
            if (ImGui::TreeNode("Style")) {
                EditColor("Color", p.color);
                ImGui::DragFloat("Radius", &p.visualRadius, 0.01f);
                ImGui::Checkbox("Show Label", &p.showLabel);
                if (p.showLabel) EditColor("Text Color", p.textColor);
                ImGui::TreePop();
            }
            ImGui::PopID();
            ImGui::Separator();
        }
        
        // Handle Deletion after iteration
        if (point_to_delete != -1) DeletePoint(entities, point_to_delete);
    }

    // ==========================================
    // 4. MEASUREMENTS (ANGLES)
    // ==========================================
    if (ImGui::CollapsingHeader("Measurements")) {
        if (ImGui::Button("Add Angle Measure")) entities.angles.push_back(AngleMeas());
        
        int remove_ang = -1;
        for (int i = 0; i < (int)entities.angles.size(); i++) {
            ImGui::PushID(i + 9000);
            if (ImGui::TreeNode("Angle", "Angle %d", i)) {
                PointSelector("Start", &entities.angles[i].p1, entities.points);
                PointSelector("Vertex", &entities.angles[i].vertex, entities.points);
                PointSelector("End", &entities.angles[i].p2, entities.points);
                
                ImGui::DragFloat("Radius", &entities.angles[i].radius, 0.1f);
                ImGui::DragFloat("Thickness", &entities.angles[i].thickness, 0.01f);
                
                EditColor("Color", entities.angles[i].color);
                if (ImGui::Button("Delete")) remove_ang = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_ang != -1) entities.angles.erase(entities.angles.begin() + remove_ang);
    }

    // ==========================================
    // 5. LINES & RAYS
    // ==========================================
    if (ImGui::CollapsingHeader("Lines & Rays")) {
        if (ImGui::Button("Add Segment")) entities.segments.push_back(Segment()); ImGui::SameLine();
        if (ImGui::Button("Add Line")) entities.lines.push_back(Line()); ImGui::SameLine();
        if (ImGui::Button("Add Ray")) entities.rays.push_back(Ray());

        // Segments
        int remove_seg = -1;
        for (int i = 0; i < (int)entities.segments.size(); i++) {
            ImGui::PushID(i);
            if (ImGui::TreeNode("Seg", "Segment %d", i)) {
                PointSelector("Start", &entities.segments[i].point1, entities.points);
                PointSelector("End", &entities.segments[i].point2, entities.points);
                EditColor("Color", entities.segments[i].color);
                EditLineStyle(entities.segments[i].thickness, entities.segments[i].dashLen, entities.segments[i].gapLen);
                if (ImGui::Button("Delete")) remove_seg = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_seg != -1) entities.segments.erase(entities.segments.begin() + remove_seg);

        // Lines
        int remove_line = -1;
        for (int i = 0; i < (int)entities.lines.size(); i++) {
            ImGui::PushID(i + 1000);
            if (ImGui::TreeNode("Line", "Line %d", i)) {
                PointSelector("Pt 1", &entities.lines[i].point1, entities.points);
                PointSelector("Pt 2", &entities.lines[i].point2, entities.points);
                EditColor("Color", entities.lines[i].color);
                EditLineStyle(entities.lines[i].thickness, entities.lines[i].dashLen, entities.lines[i].gapLen);
                if (ImGui::Button("Delete")) remove_line = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_line != -1) entities.lines.erase(entities.lines.begin() + remove_line);

        // Rays
        int remove_ray = -1;
        for (int i = 0; i < (int)entities.rays.size(); i++) {
            ImGui::PushID(i + 2000);
            if (ImGui::TreeNode("Ray", "Ray %d", i)) {
                PointSelector("Start", &entities.rays[i].point1, entities.points);
                PointSelector("Pass", &entities.rays[i].point2, entities.points);
                EditColor("Color", entities.rays[i].color);
                EditLineStyle(entities.rays[i].thickness, entities.rays[i].dashLen, entities.rays[i].gapLen);
                if (ImGui::Button("Delete")) remove_ray = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_ray != -1) entities.rays.erase(entities.rays.begin() + remove_ray);
    }

    // ==========================================
    // 6. SURFACES
    // ==========================================
    if (ImGui::CollapsingHeader("Surfaces")) {
        if (ImGui::Button("Add Surface")) entities.surfaces.push_back(Surface());
        
        int remove_surf = -1;
        for (int i = 0; i < (int)entities.surfaces.size(); i++) {
            Surface& surf = entities.surfaces[i];
            ImGui::PushID(i + 3000);
            if (ImGui::TreeNode("Surf", "Surface %d", i)) {
                EditColor("Fill", surf.surfaceColor); 
                EditColor("Border", surf.borderColor);
                EditLineStyle(surf.borderThickness, surf.dashLen, surf.gapLen);
                
                // Variable number of points
                if (ImGui::Button("Add Ref")) surf.pointIndices.push_back(0); ImGui::SameLine();
                if (surf.pointIndices.size() > 0 && ImGui::Button("Rem Ref")) surf.pointIndices.pop_back();
                
                for (size_t k = 0; k < surf.pointIndices.size(); k++) {
                    ImGui::PushID((int)k); 
                    PointSelector("Pt", &surf.pointIndices[k], entities.points); 
                    ImGui::PopID();
                }

                if (ImGui::Button("Delete")) remove_surf = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_surf != -1) entities.surfaces.erase(entities.surfaces.begin() + remove_surf);
    }

    // ==========================================
    // 7. CIRCLES & PLANES
    // ==========================================
    if (ImGui::CollapsingHeader("Circles & Planes")) {
        if (ImGui::Button("Add Circle")) entities.circles.push_back(Circle()); ImGui::SameLine();
        if (ImGui::Button("Add Plane")) entities.planes.push_back(Plane());

        // Circles
        int remove_circ = -1;
        for (int i = 0; i < (int)entities.circles.size(); i++) {
            ImGui::PushID(i + 4000);
            if (ImGui::TreeNode("Circ", "Circle %d", i)) {
                PointSelector("Mid", &entities.circles[i].midpoint, entities.points);
                PointSelector("Edge", &entities.circles[i].edgepoint, entities.points);
                PointSelector("Norm", &entities.circles[i].normpoint, entities.points);
                EditColor("Fill", entities.circles[i].surfaceColor);
                EditColor("Border", entities.circles[i].borderColor);
                EditLineStyle(entities.circles[i].thickness, entities.circles[i].dashLen, entities.circles[i].gapLen);
                if (ImGui::Button("Delete")) remove_circ = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_circ != -1) entities.circles.erase(entities.circles.begin() + remove_circ);

        // Planes
        int remove_plane = -1;
        for (int i = 0; i < (int)entities.planes.size(); i++) {
            ImGui::PushID(i + 5000);
            if (ImGui::TreeNode("Plane", "Plane %d", i)) {
                PointSelector("P1", &entities.planes[i].point1, entities.points);
                PointSelector("P2", &entities.planes[i].point2, entities.points);
                PointSelector("P3", &entities.planes[i].point3, entities.points);
                EditColor("Color", entities.planes[i].color);
                if (ImGui::Button("Delete")) remove_plane = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_plane != -1) entities.planes.erase(entities.planes.begin() + remove_plane);
    }

    // ==========================================
    // 8. 3D SHAPES
    // ==========================================
    if (ImGui::CollapsingHeader("3D Shapes")) {
        if (ImGui::Button("Add Sphere")) entities.spheres.push_back(Sphere()); ImGui::SameLine();
        if (ImGui::Button("Add Cylinder")) entities.cylinders.push_back(Cylinder()); ImGui::SameLine();
        if (ImGui::Button("Add Cone")) entities.cones.push_back(Cone());

        // Spheres
        int remove_sph = -1;
        for (int i = 0; i < (int)entities.spheres.size(); i++) {
            ImGui::PushID(i + 6000);
            if (ImGui::TreeNode("Sph", "Sphere %d", i)) {
                PointSelector("Center", &entities.spheres[i].midpoint, entities.points);
                PointSelector("Radius P", &entities.spheres[i].edgepoint, entities.points);
                EditColor("Body", entities.spheres[i].sphereColor);
                EditColor("Ring", entities.spheres[i].ringColor);
                EditLineStyle(entities.spheres[i].ringThickness, entities.spheres[i].dashLen, entities.spheres[i].gapLen);
                if (ImGui::Button("Delete")) remove_sph = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_sph != -1) entities.spheres.erase(entities.spheres.begin() + remove_sph);

        // Cylinders
        int remove_cyl = -1;
        for (int i = 0; i < (int)entities.cylinders.size(); i++) {
            ImGui::PushID(i + 7000);
            if (ImGui::TreeNode("Cyl", "Cylinder %d", i)) {
                PointSelector("Bot", &entities.cylinders[i].midpoint, entities.points);
                PointSelector("Edge", &entities.cylinders[i].edgepoint, entities.points);
                PointSelector("Top", &entities.cylinders[i].toppoint, entities.points);
                EditColor("Body", entities.cylinders[i].cylinderColor);
                EditColor("Lines", entities.cylinders[i].outlineColor);
                EditLineStyle(entities.cylinders[i].outlineThickness, entities.cylinders[i].dashLen, entities.cylinders[i].gapLen);
                if (ImGui::Button("Delete")) remove_cyl = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_cyl != -1) entities.cylinders.erase(entities.cylinders.begin() + remove_cyl);

        // Cones
        int remove_cone = -1;
        for (int i = 0; i < (int)entities.cones.size(); i++) {
            ImGui::PushID(i + 8000);
            if (ImGui::TreeNode("Cone", "Cone %d", i)) {
                PointSelector("Bot", &entities.cones[i].midpoint, entities.points);
                PointSelector("Edge", &entities.cones[i].edgepoint, entities.points);
                PointSelector("Tip", &entities.cones[i].toppoint, entities.points);
                EditColor("Body", entities.cones[i].coneColor);
                EditColor("Lines", entities.cones[i].outlineColor);
                EditLineStyle(entities.cones[i].outlineThickness, entities.cones[i].dashLen, entities.cones[i].gapLen);
                if (ImGui::Button("Delete")) remove_cone = i;
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_cone != -1) entities.cones.erase(entities.cones.begin() + remove_cone);
    }
    
    ImGui::End();
}

// -----------------------------------------------------------------------------
// RENDER LOOP
// -----------------------------------------------------------------------------

void RenderScene(Renderer* renderer, Camera* cam, const DrawEntities& entities) {
    if (!renderer || !cam) return;
    auto GetPos = [&](int idx) -> glm::vec3 {
        if (idx >= 0 && idx < (int)entities.points.size()) return entities.points[idx].pos;
        return glm::vec3(0.0f);
    };

    // 1. Points
    for (const auto& p : entities.points) {
        glm::vec3 radiusPoint = p.pos + glm::vec3(p.visualRadius, 0.0f, 0.0f);
        // glm::u8vec4 ringCol = (p.op != OP_FREE) ? glm::u8vec4{0,255,255,255} : glm::u8vec4{0,0,0,0};
        DrawSphere(renderer, cam, p.pos, radiusPoint, 8, 8, p.color, {0,0,0,0}, 0.05f, 0.0f, 0.0f);
        if (p.showLabel) {
            glm::vec3 textPos = p.pos;
            textPos.y += p.visualRadius + entities.globalTextOffset; 
            DrawText(renderer, cam, p.name.c_str(), textPos, entities.globalTextSize, p.textColor);
        }
    }
    
    // 2. Lines & Segments
    for (const auto& l : entities.lines) 
        DrawLine(renderer, GetPos(l.point1), GetPos(l.point2), l.color, l.thickness, l.dashLen, l.gapLen);
    for (const auto& r : entities.rays) 
        DrawRay(renderer, GetPos(r.point1), GetPos(r.point2), r.color, r.thickness, r.dashLen, r.gapLen);
    for (const auto& s : entities.segments) {
        glm::vec3 pts[2] = { GetPos(s.point1), GetPos(s.point2) };
        DrawSegment(renderer, pts, 2, s.thickness, s.color, s.dashLen, s.gapLen);
    }
    
    // 3. Angles
    for (const auto& a : entities.angles) {
        // Pass Global Text Size and Offset
        DrawAngle(renderer, cam, 
                  GetPos(a.p1), GetPos(a.vertex), GetPos(a.p2), 
                  a.color, a.radius, a.thickness, 
                  entities.globalTextSize, entities.globalTextOffset);
    }

    // 4. Other Shapes
    for (const auto& surf : entities.surfaces) {
        if (surf.pointIndices.size() < 3) continue;
        std::vector<glm::vec3> polyPoints; polyPoints.reserve(surf.pointIndices.size());
        for (int idx : surf.pointIndices) polyPoints.push_back(GetPos(idx));
        DrawSurface(renderer, polyPoints.data(), (int)polyPoints.size(), surf.surfaceColor, surf.borderColor, surf.borderThickness, surf.dashLen, surf.gapLen);
    }
    for (const auto& p : entities.planes) 
        DrawPlane(renderer, GetPos(p.point1), GetPos(p.point2), GetPos(p.point3), p.color);
    for (const auto& c : entities.circles) 
        DrawCircle(renderer, GetPos(c.midpoint), GetPos(c.edgepoint), GetPos(c.normpoint), c.steps, c.surfaceColor, c.borderColor, c.thickness, c.dashLen, c.gapLen);
    for (const auto& s : entities.spheres) 
        DrawSphere(renderer, cam, GetPos(s.midpoint), GetPos(s.edgepoint), s.sectorCount, s.stackCount, s.sphereColor, s.ringColor, s.ringThickness, s.dashLen, s.gapLen);
    for (const auto& c : entities.cylinders) 
        DrawCylinder(renderer, cam, GetPos(c.midpoint), GetPos(c.toppoint), GetPos(c.edgepoint), c.sectorCount, c.cylinderColor, c.outlineColor, c.outlineThickness, c.dashLen, c.gapLen);
    for (const auto& c : entities.cones) 
        DrawCone(renderer, cam, GetPos(c.midpoint), GetPos(c.toppoint), GetPos(c.edgepoint), c.sectorCount, c.coneColor, c.outlineColor, c.outlineThickness, c.dashLen, c.gapLen);
}