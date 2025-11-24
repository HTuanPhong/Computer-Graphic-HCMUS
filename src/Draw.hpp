#ifndef DRAW_H
#define DRAW_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Render.hpp"
#include "Camera.hpp"

struct Point
{
  glm::vec3 pos;
};

struct Line 
{
	int point1;
	int point2;
};

struct Segment
{
	int point1;
	int point2;
};

struct Ray
{
	int point1;
	int point2;
};

struct Surface
{
	int point1;
	int point2;
	int point3;
};

struct Plane
{
	int point1;
	int point2;
	int point3;
};

struct Circle
{
	int midpoint;
	int edgepoint;
	int normpoint;
};

struct Sphere
{
	int midpoint;
	int edgepoint;
};

struct Cylinder
{
	int midpoint;
	int edgepoint;
	int toppoint;
};

struct Cone
{
	int midpoint;
	int edgepoint;
	int toppoint;
};

void DrawText(Renderer* renderer, Camera* cam, const char* text, glm::vec3 pos, float size, glm::u8vec4 color);
void DrawSphere(Renderer *renderer, glm::vec3 center, float radius, int sectorCount,  int stackCount, glm::u8vec4 color);
void DrawCylinder(Renderer *renderer, glm::vec3 bottom);
#endif