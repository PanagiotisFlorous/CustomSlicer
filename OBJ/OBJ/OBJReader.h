#ifndef _OBJ_READER
#define _OBJ_READER
#include <iostream>
#include <vector>
#include <fstream>
#include <utility>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include "glm/vec3.hpp"
#include "glm/glm.hpp"
#include "Plane.h"
#include "Point.h"
#include "Face.h"

class OBJReader {
private:
	std::vector<Point> vertices;
	std::vector<Face> faces;
	std::map<int,int> order;
	std::pair<float, float> minMax;
public:
	OBJReader(std::string filepath);
	OBJReader(std::string filepath, Plane p);
	std::string faceCleanup(std::string faceString);
	void minMaxCalc(Plane p);
	int getThreshold(Plane p);
	std::vector<Point> getVertices();
	Point getVertex(int i);
	std::vector<Face> getFaces();
	std::map<int, int> getOrder();
	std::pair<float, float> getMinMax();
};
#endif