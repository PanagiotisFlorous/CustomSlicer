#ifndef _FACE
#define _FACE
#include <vector>
#include <algorithm>
#include "glm/vec3.hpp"
#include "glm/glm.hpp"

class Face {
private:
	glm::vec3 verticesIDX;
	std::vector<float*> verticesDist;
public:
	Face(glm::vec3 vertices, std::vector<float*> distances);
	std::vector<int> getVertices();
	std::vector<float> getDist();
	float getMin() const;
};
#endif
