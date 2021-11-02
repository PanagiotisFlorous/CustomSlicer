#include "Face.h"

//Class representing the faces in a model.
Face::Face(glm::vec3 vertices, std::vector<float*> distances) {
	verticesIDX = vertices;
	verticesDist = distances;
}

//getters for the vertices and distances
std::vector<int> Face::getVertices() {
	return std::vector<int> {(int)verticesIDX.x, (int)verticesIDX.y, (int)verticesIDX.z};
}

std::vector<float> Face::getDist() {
	std::vector<float> ret = { *verticesDist[0], *verticesDist[1], *verticesDist[2] };
	return ret;
}

//helper fucntion to save time when sorting faces. Returns the distance of the minimum point in the face.
float Face::getMin() const{
	//std::vector<float> ret = { *verticesDist[0], *verticesDist[1], *verticesDist[2] };
	//return *std::min_element(ret.begin(), ret.end());
	float min = std::numeric_limits<float>::max();
	int minid;
	for (size_t i = 0; i < 3; i++)
	{
		if (*verticesDist[i] < min) {
			min = *verticesDist[i];
			minid = i;
		}
	}
	return *verticesDist[minid];
}