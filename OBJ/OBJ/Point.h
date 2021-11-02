#ifndef _POINT
#define _POINT
#include "glm/vec3.hpp"
#include "glm/glm.hpp"
#include "Plane.h"
class Point {
private:
	glm::vec3 coordinates;
	float distance;
public:
	Point(glm::vec3 point);
	Point(glm::vec3 point, Plane p);
	void setDist(float dist);
	glm::vec3 getCoord();
	float getDist();
	float* getPointer();
};
#endif
