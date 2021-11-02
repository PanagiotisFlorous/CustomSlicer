#include "Point.h"

//Class for point representation.
Point::Point(glm::vec3 point) {
	coordinates = point;
	distance = 0.0f;
}

Point::Point(glm::vec3 point, Plane p) : Point(point) {
	distance = (p.normal.x * point.x) + (p.normal.y * point.y) + (p.normal.z * point.z);
}

//Simple setter and getters.
void Point::setDist(float dist) {
	distance = dist;
}

glm::vec3 Point::getCoord() {
	return coordinates;
}

float Point::getDist() {
	return distance;
}

float* Point::getPointer() {
	return &distance;
}