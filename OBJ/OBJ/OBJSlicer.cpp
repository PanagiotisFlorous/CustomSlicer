#include "OBJSlicer.h"
//Class for the custom slicer.

//Custom comparator to be used in binary search later.
bool customComp(Face f,float bound) {
	std::vector<float> f1 = f.getDist();
	return *std::min_element(f1.begin(), f1.end()) > bound;
}

//Calculates the optimal velocity relative to curvature.
float velocity(float curvature) {
	float radius = 1000 / curvature;

	//This function was calculated by Dr.Ryuji Hirayama according to experiments done when writing this program. If you are modifying this program this probably needs to change.
	return 3.8407778 - (3.7139906 * exp(-0.66317368*radius));
}

//Function to calculate the angle made by 3 points. Probably obsolete.
float angleBetween(glm::vec3 left, glm::vec3 middle, glm::vec3 right) {
	glm::vec3 LtoM = glm::normalize(left - middle);
	glm::vec3 RtoM = glm::normalize(right - middle);

	return glm::acos(glm::dot(LtoM, RtoM));
}

//Function that returns the curvature between 3 points. Initially it returned the radius of a circle passing through all 3 points. To make it do that simply return 1/curvature.
float radius(glm::vec3 left, glm::vec3 middle, glm::vec3 right) {
	//glm::vec3 n = glm::cross(middle - left, right - middle);
	//glm::vec3 mid1 = 0.5f*(left + middle);
	//glm::vec3 mid2 = 0.5f*(right + middle);
	//glm::vec3 dp0 = glm::cross(middle - left, n);
	//glm::vec3 dp1 = glm::cross(right - middle, n);
	////float v = ((mid2.x * dp0.y)/* - (mid2.y * dp0.x) + (dp0.x * mid1.y) - (dp0.y * mid1.x)*/);
	//float u = ((mid2.x*dp0.y) - (mid2.y*dp0.x) + (dp0.x*mid1.y) - (dp0.y*mid1.x)) / ((dp0.x * dp1.y) - (dp0.y - dp1.x));
	//glm::vec3 centre = mid2 + (dp1 * u);

	//std::cout << u << " " << u << " " << mid1.z << std::endl;

	//return glm::distance(centre, left);
	float costheta = glm::dot(left - middle, left - right) / (glm::distance(left, middle) * glm::distance(left, right));
	float theta = acos(costheta);
	float area = 0.5f * (glm::distance(left, middle) * glm::distance(left, right)) * sin(theta);

	float curvature = (4 * area) / (glm::distance(left, middle) * glm::distance(left, right) * glm::distance(middle, right));

	return curvature;

}

//Constructors when you have a file and a sliceplane or an OBJReader object and a sliceplane.
OBJSlicer::OBJSlicer(std::string filepath, Plane p) {
	obj = new OBJReader(filepath, p);
	slicePlane = p;
	pathLength = 0.0f;
}

OBJSlicer::OBJSlicer(OBJReader* model, Plane p) {
	obj = model;
	obj->minMaxCalc(p);
	slicePlane = p;
	pathLength = 0.0f;
}

//Makes the path for the GCode
void OBJSlicer::makePath(int slicesNum, int startSlice, int endSlice) {
	//Change implementation with CUDA 
	//Helper vector. Remove if solution is found to get the faces immideately in the list.
	std::vector<Face> temp = obj->getFaces();
	std::list<Face> tempFaces(temp.begin(), temp.end());
	//Remove if sorting is reversed.
	tempFaces.reverse();
	//Possibly obsolete. Remove from both classes after test.
	std::map<int, int> tempOrder = obj->getOrder();
	//previous endpoint of path. Used for optimization in getStep()
	int ppIt = 0;
	//Add 2 slices to start at slice 2 and end at slice n-1.
	//It does not affect the detail much and removes cases where a slice has 1 or more non-connected points.
	float lowBound = obj->getMinMax().first;
	float highBound = obj->getMinMax().second;
	float step = (highBound - lowBound) / (slicesNum + 2);
	lowBound += (startSlice + 1)*step;
	highBound -= (slicesNum - endSlice + 1)*step;
	slicePlane.direction = lowBound;
	//Variables to find the closest point
	int endIdx = 0;
	float endDist;
	//Loop through all the slices.
	while (lowBound < highBound) {
		endDist = std::numeric_limits<float>::max();
		//Debug test. Remove before release.
		//std::cout << lowBound << std::endl;
		//Get the point closest to, but below the lowBound.
		std::list<Face>::iterator it;
		it = std::lower_bound(tempFaces.begin(), tempFaces.end(), lowBound, customComp);
		//possibly remove
		//int faceIdx = it - tempFaces.begin();
		while (it != tempFaces.end()/*(slicePlane.getFaceWeight(tempFaces[faceIdx])) > -3 possibly remove*/) {
			//If not all points are below the plane process the face. If they are remove it.
			//We remove it to reduce the search space. If it is under the current plane, it is under every plane to come.
			if ((slicePlane.getFaceWeight(*it)) > -3) {
				getStep(it->getVertices(), ppIt);
				it++;
			}
			else {
				it = tempFaces.erase(it);
			}
		}
		float tempDist;
		int newIdx = 0;
		for (std::vector<glm::vec3>::iterator it = pathPoints.begin() + ppIt; it != pathPoints.end(); it++) {
			tempDist = glm::distance(pathPoints[endIdx], *it);
			if (tempDist < endDist) {
				endDist = tempDist;
				newIdx = it - pathPoints.begin();
			}
		}
		for (size_t i = 0; i < path.size() / 10; i++)
		{
			newIdx = path[newIdx];
		}
		//endIdx = newIdx;		
		std::cout << endIdx << std::endl;
		//Debug test. Remove before release.
		//d::cout << tempFaces.size() << std::endl;
		//Call gcode generator, clear the path, iterate.
		generateCurvature(newIdx);
		generateGCode(newIdx, endIdx);
		endIdx = newIdx;
		//generateOBJ();
		path.clear();
		velocityVec.clear();
		lowBound += step;
		slicePlane.direction = lowBound;
		ppIt = pathPoints.end() - pathPoints.begin();
	}

}

//Adds a line to the layer.
inline void OBJSlicer::getStep(std::vector<int> faceVector, int ppIt) {
	//Find the "lone" vertex.
	std::vector<int> order = { 0, 1, 2 };
	while ((slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[0]]).getCoord()) == slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[1]]).getCoord())) || (slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[0]]).getCoord()) == slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[2]]).getCoord()))) {
		//debugging outputs remove if necessary.
		//std::cout << slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[0]]).getCoord()) << std::endl;
		//std::cout << slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[1]]).getCoord()) << std::endl;
		//std::cout << slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[2]]).getCoord()) << std::endl;

		rotate(order.begin(), order.begin() + 1, order.end());
	}
	//Get first intersection. If statement is used to preserve orientation of the lines processed throughout the model.
	//It is necessary to check for duplicate points later. Not doing so introduces error due to the nature of floats.
	glm::vec3 firstInt;
	if (slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[0]]).getCoord()) > 0) {
		firstInt = slicePlane.intersectionPoint(obj->getVertex(faceVector[order[0]]).getCoord(), obj->getVertex(faceVector[order[1]]).getCoord());
	}
	else {
		firstInt = slicePlane.intersectionPoint(obj->getVertex(faceVector[order[1]]).getCoord(), obj->getVertex(faceVector[order[0]]).getCoord());
	}
	//Check if this is a duplicate vertex.
	int firstIntIndex = std::find_if(pathPoints.begin() + ppIt, pathPoints.end(), [&](glm::vec3 const& v1) {
		float dist = glm::distance(v1, firstInt);
		return dist < 0.0000000001f;
		}) - pathPoints.begin();
		//If it's not, add it.
		if (firstIntIndex == (pathPoints.end() - pathPoints.begin())) {
			pathPoints.push_back(firstInt);
			firstIntIndex = pathPoints.size() - 1;
		}
		glm::vec3 secondInt;
		if (slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[0]]).getCoord()) > 0) {
			secondInt = slicePlane.intersectionPoint(obj->getVertex(faceVector[order[0]]).getCoord(), obj->getVertex(faceVector[order[2]]).getCoord());
		}
		else {
			secondInt = slicePlane.intersectionPoint(obj->getVertex(faceVector[order[2]]).getCoord(), obj->getVertex(faceVector[order[0]]).getCoord());
		}
		//Same procedure as above.
		int secondIntIndex = std::find_if(pathPoints.begin() + ppIt, pathPoints.end(), [&](glm::vec3 const& v1) {
			float dist = glm::distance(v1, secondInt);
			return dist < 0.0000000001f;
			}) - pathPoints.begin();
			if (secondIntIndex == (pathPoints.end() - pathPoints.begin())) {
				pathPoints.push_back(secondInt);
				secondIntIndex = pathPoints.size() - 1;
			}
			//This is the case where it finds the same point twice. This should never happen in theory. Usually when it happens it does not affect the program, but if it happens a lot, change the
			//threshold's accuracy when checking for duplicates above.
			if (firstIntIndex == secondIntIndex) {
				std::cout << "test"  << std::endl;
			}
			//Similar to above if, preserves the orientation of our newly made lines.
			if (slicePlane.pointAbovePlane(obj->getVertex(faceVector[order[0]]).getCoord()) > 0) {
				path[secondIntIndex] = firstIntIndex;
			}
			else {
				path[firstIntIndex] = secondIntIndex;
			}
}

//Generates the GCode.
void OBJSlicer::generateGCode(int pathStart, int prevEnd) {
	std::map<int, int> tempPath = path;
	bool first = true;
	//account for multiple closed paths
	pathLength += glm::distance(pathPoints[pathStart], pathPoints[prevEnd]);
	while (tempPath.size() > 0) {
		//int start = tempPath.begin()->first;
		//int step = tempPath.begin()->second;
		int velIdx = 0;
		int start;
		int step;
		if (first) {
			start = pathStart;
			step = tempPath[start];
			first = false;
		}
		else {
			start = tempPath.begin()->first;
			step = tempPath.begin()->second;
		}
		step = tempPath[start];
		tempPath.erase(start);
		gcode << "G0 " << "X" << pathPoints[start].x*10 << " Y" << pathPoints[start].y * 10 << " Z" << pathPoints[start].z * 10 << " F"<< velocityVec[velIdx]<< std::endl;
		pathLength += glm::distance(pathPoints[start], pathPoints[step]);
		while (start != step) {
			//debugging output.
			/*std::cout << start << " " << step << std::endl;
			if (step == 0) {
				break;
			}*/
			velIdx++;
			gcode << "G1 " << "X" << pathPoints[step].x * 10 << " Y" << pathPoints[step].y * 10 << " Z" << pathPoints[step].z * 10 << " F" << velocityVec[velIdx] << std::endl;
			int prevStep = step;
			step = tempPath[step];
			pathLength += glm::distance(pathPoints[prevStep], pathPoints[step]);
			tempPath.erase(prevStep);
		}
		gcode << "G1 " << "X" << pathPoints[start].x * 10 << " Y" << pathPoints[start].y * 10 << " Z" << pathPoints[start].z * 10 << std::endl;
	}
	
}

//Method to generate a relationship between points and curvature. Functions the same as the generateGCode function, but skips a constant number of points in the path for each iteration.
void OBJSlicer::generateCurvature(int pathStart) {
	std::map<int, int> tempPath = path;
	std::map<int, int> basePath = path;
	std::vector<float> curvature;
	bool first = true;
	while (tempPath.size() > 0) {
		int start;
		int left, middle, right;
		if (first) {
			start = pathStart;
			left = pathStart;
			middle = tempPath[left];
			right = tempPath[middle];
			first = false;
		}
		else {
			start = tempPath.begin()->first;
			left = tempPath.begin()->first;
			middle = tempPath[left];
			right = tempPath[middle];
		}
		if (path.size() > 20) {
			for (size_t i = 0; i < 7; i++)
			{
				middle = tempPath[middle];
			}
			right = middle;
			for (size_t i = 0; i < 8; i++)
			{
				right = tempPath[right];
			}
		}
		curvature.push_back(radius(pathPoints[left], pathPoints[middle], pathPoints[right]));
		left = tempPath[left];
		middle = tempPath[middle];
		right = tempPath[right];
		tempPath.erase(left);
		while (start != left) {
			curvature.push_back(radius(pathPoints[left], pathPoints[middle], pathPoints[right]));
			left = basePath[left];
			middle = basePath[middle];
			right = basePath[right];
			tempPath.erase(left);
		}
		for (std::vector<float>::iterator it = curvature.begin(); it != curvature.end(); it++) {
			//std::cout << velocity(*it) << std::endl;
			if (!isnan(*it)) {
				velocityVec.push_back(velocity(*it));
			}
			else {
				//change to max velocity
				velocityVec.push_back(100.0f);
			}
			//velocityVec.push_back(1000 / (*it));
		}
	}

}
//Old implementation of above function. Leaving it in as it might be useful to you.
//
//void OBJSlicer::generateCurvature(int pathStart) {
//	std::map<int, int> tempPath = path;
//	std::map<int, int> basePath = path;
//	std::vector<float> curvature;
//	//account for multiple closed paths
//	while (tempPath.size() > 0) {
//		int start;
//		int left, middle, right;
//		start = pathStart;
//		left = pathStart;
//		middle = tempPath[left];
//		right = tempPath[middle];
//		//tempPath.erase(start);
//		for (size_t i = 0; i < 7; i++)
//		{
//			middle = tempPath[middle];
//		}
//		right = middle;
//		for (size_t i = 0; i < 8; i++)
//		{
//			right = tempPath[right];
//		}
//		curvature.push_back(radius(pathPoints[left], pathPoints[middle], pathPoints[right]));
//		left = tempPath[left];
//		middle = tempPath[middle];
//		right = tempPath[right];
//		while (start != left) {
//			/*left = tempPath[left];
//			middle = tempPath[middle];
//			right = tempPath[right];
//			curvature.push_back(radius(pathPoints[left], pathPoints[middle], pathPoints[right]));*/
//			curvature.push_back(radius(pathPoints[left], pathPoints[middle], pathPoints[right]));
//			left = tempPath[left];
//			middle = tempPath[middle];
//			right = tempPath[right];
//		}
//		std::vector<float> curvatureDer;
//		for (size_t i = 0; i < curvature.size()-1; i++)
//		{
//			curvatureDer.push_back(curvature[i] - curvature[i+1]);
//		}
//		for (std::vector<float>::iterator it = curvature.begin(); it != curvature.end(); it++) {
//			float f = *it;
//			std::cout << 1000/f << std::endl;
//			
//			//std::cout << "radius:" <<  1000/(*it) << std::endl;
//			//velocityVec.push_back(velocity(*it));
//			velocityVec.push_back(1000/(*it));
//
//		}
//		break;
//	}
//	
//}

//Generates and obj-like structure of our sliced model to be able to render it. Not sure if it works 100% check if needed.
void OBJSlicer::generateOBJ() {
	std::map<int, int> tempPath = path;
	//account for multiple closed paths
	while (tempPath.size() > 0) {
		int start = tempPath.begin()->first;
		int step = tempPath.begin()->second;
		tempPath.erase(start);
		objpath << "l " << start;
		while (start != step) {
			objpath << " " << step;
			int prevStep = step;
			step = tempPath[step];
			tempPath.erase(prevStep);
		}
		objpath << std::endl;
	}
}
//Simple getters.
float OBJSlicer::getPathLength() {
	return pathLength;
}

std::stringstream& OBJSlicer::getGCode() {
	return gcode;
}

std::stringstream& OBJSlicer::getObjPath() {
	return objpath;
}