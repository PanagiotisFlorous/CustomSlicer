#ifndef _OBJ_SLICER
#define _OBJ_SLICER
#include <map>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <algorithm>
#include "OBJReader.h"
#include "Plane.h"

class OBJSlicer {
private:
	std::vector<glm::vec3> pathPoints;
	std::vector<float> velocityVec;
	std::map<int, int> path;
	std::stringstream gcode, objpath;
	float pathLength;
	Plane slicePlane;
	OBJReader* obj;
public:
	OBJSlicer(std::string filepath, Plane p);
	OBJSlicer(OBJReader* model, Plane p);
	void makePath(int slicesNum, int startSlice, int endSlice);
	inline void getStep(std::vector<int> faceVector, int ppIt);
	void generateGCode(int pathStart, int prevEnd);
	void generateOBJ();
	void generateCurvature(int pathStart);
	float getPathLength();
	std::stringstream& getGCode();
	std::stringstream& getObjPath();

};
#endif
