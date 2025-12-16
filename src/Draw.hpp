#ifndef DRAW_H
#define DRAW_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include "Render.hpp"
#include "Camera.hpp"


// --- Construction Operations ---
enum Operation {
    OP_FREE,            // User drags manually
    OP_DILATE,          // Parents: [Target, Center]. Param: Scale.
    OP_TRANSLATE,       // Parents: [Target, VecStart, VecEnd]. 
    OP_ROTATE_LINE,     // Parents: [Target, AxisP1, AxisP2]. Param: Angle.
    OP_REFLECT_PLANE,   // Parents: [Target, PlnP1, PlnP2, PlnP3].
    OP_ANGLE_DIV,       // Parents: [Start, Vertex, End]. Param: Ratio.
    OP_PROJECT_LINE,    // Parents: [Target, LineP1, LineP2]. (Drop Perpendicular)
    OP_INTERSECT_LINES, // Parents: [L1_P1, L1_P2, L2_P1, L2_P2]. (Intersection of 2 infinite lines)
    OP_PERP_TO_PLANE    // Parents: [Target, Pln1, Pln2, Pln3]. Param: Distance.
};
// --- Entities ---

struct Point {
    glm::vec3 pos = {0,0,0};
    std::string name = "Point";
    bool fixed = false;

    // Dependency Graph Data
    Operation op = OP_FREE;
    std::vector<int> parents;  // Indices of points defining this one
    std::vector<int> children; // Indices of points depending on this one
    float param = 0.0f;        // Scalar value (Angle, Scale ratio, etc.)

    // Point Visuals
    float visualRadius = 0.05f; 
    glm::u8vec4 color = {0, 0, 0, 255}; 
    
    // Text Visuals
    bool showLabel = true;
    glm::u8vec4 textColor = {0,0,0, 255};
};

struct Line {
    int point1 = 0;
    int point2 = 0;
    glm::u8vec4 color = {0,0,0, 255};
    float thickness = 0.01f;
    float dashLen = 0.05f;
    float gapLen = 0.05f;
};

typedef Line Segment; 
typedef Line Ray;

struct AngleMeas {
    int p1 = 0;     // Start
    int vertex = 0; // Vertex
    int p2 = 0;     // End
    float radius = 0.3f;
    float thickness = 0.01f;
    glm::u8vec4 color = {0,0,0, 255};
};


struct Surface {
    std::vector<int> pointIndices; 
    glm::u8vec4 surfaceColor = {0, 255, 0, 50};
    
    // Border Style
    glm::u8vec4 borderColor = {0, 0, 0, 255};
    float borderThickness = 0.01f;
    float dashLen = 0.05f;
    float gapLen = 0.05f;
};

struct Plane {
    int point1 = 0;
    int point2 = 0;
    int point3 = 0;
    glm::u8vec4 color = {100, 100, 255, 50};
};

struct Circle {
    int midpoint = 0;
    int edgepoint = 0;
    int normpoint = 0;
    int steps = 32;
    
    glm::u8vec4 surfaceColor = {255, 0, 0, 50};
    glm::u8vec4 borderColor = {0, 0, 0, 255};
    float thickness = 0.01f;
    float dashLen = 0.05f;
    float gapLen = 0.06f;
};

struct Sphere {
    int midpoint = 0;
    int edgepoint = 0;
    int sectorCount = 32;
    int stackCount = 32;
    
    glm::u8vec4 sphereColor = {255, 165, 0, 50};
    
    // Ring/Halo Style
    glm::u8vec4 ringColor = {0,0,0, 255};
    float ringThickness = 0.01f;
    float dashLen = 0.05f;
    float gapLen = 0.05f;
};

struct Cylinder {
    int midpoint = 0;
    int edgepoint = 0;
    int toppoint = 0;
    int sectorCount = 32;
    
    glm::u8vec4 cylinderColor = {0, 0, 255, 50};
    
    // Outline Style
    glm::u8vec4 outlineColor = {0,0,0, 255};
    float outlineThickness = 0.01f;
    float dashLen = 0.05f;
    float gapLen = 0.05f;
};

struct Cone {
    int midpoint = 0; 
    int edgepoint = 0; 
    int toppoint = 0; 
    int sectorCount = 32;
    
    glm::u8vec4 coneColor = {255, 0, 255, 50};
    
    // Outline Style
    glm::u8vec4 outlineColor = {0,0,0, 255};
    float outlineThickness = 0.01f;
    float dashLen = 0.05f;
    float gapLen = 0.05f;
};

struct DrawEntities {
    //global
    float globalTextSize = 14.0f;
    float globalTextOffset = 0.15f;
    //entity
    std::vector<Point> points;
    std::vector<Line> lines;
    std::vector<Segment> segments;
    std::vector<AngleMeas> angles;
    std::vector<Ray> rays;
    std::vector<Surface> surfaces;
    std::vector<Plane> planes;
    std::vector<Circle> circles;
    std::vector<Sphere> spheres;
    std::vector<Cylinder> cylinders;
    std::vector<Cone> cones;
};

void DrawGeometryEditor(DrawEntities& entities);
void RenderScene(Renderer* renderer, Camera* cam, const DrawEntities& entities);
void RecalculatePoint(DrawEntities& entities, int pointIdx);
void PropagateUpdates(DrawEntities& entities, int startNodeIdx);
void SaveScene(const DrawEntities& entities, const std::string& filename);
void LoadScene(DrawEntities& entities, const std::string& filename);
void DrawText(Renderer* renderer, Camera* cam, const char* text, glm::vec3 pos, float size, glm::u8vec4 color);
void DrawSegment(Renderer* renderer, glm::vec3* pos, int posSize, float radius, glm::u8vec4 color, float dashLen, float gapLen);
void DrawAngle(Renderer* renderer, const Camera* cam, 
               glm::vec3 start, glm::vec3 vertex, glm::vec3 end, 
               glm::u8vec4 color, float radius, float thickness, 
               float textSize, float textOffset);
void DrawRay(Renderer* renderer, glm::vec3 start, glm::vec3 passThrough, 
             glm::u8vec4 color, float thickness, float dashLen, float gapLen);
void DrawLine(Renderer* renderer, glm::vec3 p1, glm::vec3 p2, 
							glm::u8vec4 color, float thickness, float dashLen, float gapLen); 
void DrawSurface(Renderer* renderer, glm::vec3* points, int count, 
                 glm::u8vec4 surfaceColor, 
                 glm::u8vec4 borderColor, float thickness, float dashLen, float gapLen);
void DrawPlane(Renderer* renderer, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::u8vec4 color);
void DrawCircle(Renderer* renderer, 
                glm::vec3 midpoint, glm::vec3 edgepoint, glm::vec3 normpoint, 
                int steps, 
                glm::u8vec4 surfaceColor,
                glm::u8vec4 borderColor, float thickness, float dashLen, float gapLen);
void DrawSphere(Renderer* renderer, const Camera* cam,
                glm::vec3 center, glm::vec3 radiusPoint, 
                int sectorCount, int stackCount, glm::u8vec4 sphereColor, 
                glm::u8vec4 ringColor, float ringThickness, float ringDashLen, float ringGapLen);
void DrawCylinder(Renderer* renderer, const Camera* cam,
                  glm::vec3 bottomCenter, glm::vec3 topCenter, glm::vec3 bottomEdge, 
                  int sectorCount, glm::u8vec4 cylinderColor,
                  glm::u8vec4 outlineColor, float outlineThickness, float dashLen, float gapLen);
void DrawCone(Renderer* renderer, const Camera* cam,
              glm::vec3 bottomCenter, glm::vec3 topCenter, glm::vec3 bottomEdge, 
              int sectorCount, glm::u8vec4 coneColor,
              glm::u8vec4 outlineColor, float outlineThickness, float dashLen, float gapLen);
#endif