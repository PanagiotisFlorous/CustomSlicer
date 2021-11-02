#ifndef _PLANE
#define _PLANE

#include "glm/vec3.hpp"
#include <iostream>
#include "Face.h"


struct Plane {
	//normals and direction of the plane.
	glm::vec3 normal;
	float direction;

	Plane() {

	}

	//constructor from 3 vertices, calculates and stores normals and direction of the plane
	Plane(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
		//glm::vec3 n = (a - b) % (b - c); //perform cross product of two lines on plane
		glm::vec3 n = glm::cross(a - b, b - c);
		if (glm::length(n) > 0)
		{
			normal = glm::normalize(n);        //assign new normal to member normal
			direction = glm::dot(normal, a);   //offset plane from origin
		}
	}

	//returns the point of intersection between 2 points. CAUTION: using the same points in different order returns different results. The error margin depends on the accuracy of the original points. Not my fault, floats are a bad datatype.
	glm::vec3 intersectionPoint(glm::vec3 a, glm::vec3 b) {
		glm::vec3 helper = b - a;
		float nDotA = glm::dot(normal, a);
		float nDotH = glm::dot(normal, helper);

		return a + (((direction - nDotA) / nDotH) * helper);

	}

	// returns 0 if the given point is on the plane, 1 if above and -1 if below
	int pointAbovePlane(glm::vec3 point) {
		float formulaRes = ((normal.x * point.x) + (normal.y * point.y) + (normal.z * point.z) - direction);
		if (formulaRes == 0) {
			return 0;
		}
		else if (formulaRes > 0) {
			return 1;
		}
		else {
			return -1;
		}
	}

	int getFaceWeight(Face f) {
		std::vector<float> temp = f.getDist();
		int weight = 0;

		for (std::vector<float>::iterator it = temp.begin(); it != temp.end(); ++it) {
			if (*it - direction > 0) {
				weight++;
			}
			else {
				weight--;
			}
		}
		return weight;
	}
};
#endif