#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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