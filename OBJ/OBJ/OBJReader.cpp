#include "OBJReader.h"

//previous iteration replaced below. It is really slow as it creates copies.
//bool compareFaces(Face face1, Face face2) {
//	std::vector<float> f1 = face1.getDist();
//	std::vector<float> f2 = face2.getDist();
//	return *std::min_element(f1.begin(), f1.end()) < *std::min_element(f2.begin(), f2.end());
//}

//comparator for sort
bool compareFaces(const Face& face1, const Face& face2) {
	return face1.getMin() < face2.getMin();
}

//Constructor when only path of file is given.
OBJReader::OBJReader(std::string filepath) {
	//Open file and store all the vertices and faces.
	std::ifstream ifs(filepath);
	std::string fileLine;
	if (ifs.is_open()) {
		while (getline(ifs, fileLine)) {
			std::stringstream toParse(fileLine);
			std::vector<std::string> lineTokens;
			std::string token;
			while (std::getline(toParse, token, ' ')) {
				lineTokens.push_back(token);
			}
			if (!lineTokens.empty()) {
				if (lineTokens[0] == "v") {
					vertices.push_back(Point(glm::vec3(atof(lineTokens[1].c_str()), atof(lineTokens[2].c_str()), atof(lineTokens[3].c_str()))));
				}
				else if (lineTokens[0] == "f") {
					//Strip vector normals and textures from the faces
					lineTokens[1] = faceCleanup(lineTokens[1]);
					lineTokens[2] = faceCleanup(lineTokens[2]);
					lineTokens[3] = faceCleanup(lineTokens[3]);
					std::vector<float*> tempDist = { vertices[stoi(lineTokens[1])-1].getPointer(), vertices[stoi(lineTokens[2])-1].getPointer(), vertices[stoi(lineTokens[3])-1].getPointer() };
					faces.push_back(Face(glm::vec3(stoi(lineTokens[1]) - 1, stoi(lineTokens[2]) - 1, stoi(lineTokens[3]) - 1), tempDist));
				}
			}
		}
	}
}

//Constructor for when path of file and a plane is given to calculate the "bounding planes" of model.
//NOTE: Only C++11 supports delegate constructors, if a previous version is used copy and paste the
//above constructor's code ABOVE the method.
OBJReader::OBJReader(std::string filepath, Plane p): OBJReader(filepath) {
	minMaxCalc(p);
}

//Strips vector normals and textures from faces.
std::string OBJReader::faceCleanup(std::string faceString) {
	std::stringstream toParse(faceString);
	std::vector<std::string> lineTokens;
	std::string token;
	while (getline(toParse, token, '/')) {
		lineTokens.push_back(token);
	}

	return lineTokens[0];
}

//Finds the "bounding planes" of the model, meaning that for a vector normal describing a plane,
//assigns the minimum and maximum distance where the model's vertex was sitting on the plane.
//In addition it sorts the faces to optimize for slicing.
//NOTE: Method works assuming the 3D Plane function A*x + B*y + C*z - d = 0.
void OBJReader::minMaxCalc(Plane p) {
	std::multimap<float, int> orderMap;
	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();
	float d = 0;
	for (std::vector<Point>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
		d = (p.normal.x * it->getCoord().x) + (p.normal.y * it->getCoord().y) + (p.normal.z * it->getCoord().z);
		//orderMap[it - vertices.begin()] = d;
		it->setDist(d);
		orderMap.insert(std::pair<float,int>(d, it-vertices.begin()));
		if (d < min) {
			min = d;
		}
		if (d > max) {
			max = d;
		}
	}
	order.clear();
	size_t i = 0;
	for (std::map<float, int>::iterator it = orderMap.begin(); it != orderMap.end(); ++it) {
		order[it->second] = i;
		i++;
	}

	std::sort(faces.begin(), faces.end(), compareFaces);
	
	minMax = std::make_pair(min, max);
}

//Possibly obsolete function that used binary search to find the highest point below the slicing plane.
//Replaced by the use std::lower_bound. Remove after checking I don't use it anywhere.
int OBJReader::getThreshold(Plane p) {
	int weight;
	size_t mid, left = 0;
	size_t right = order.size();
	while (left < right) {
		mid = left + (right - left) / 2;
		weight = p.pointAbovePlane(vertices[order[mid]].getCoord()) + p.pointAbovePlane(vertices[order[mid + 1]].getCoord());
		//std::cout << weight << std::endl;
		if (weight < 0) {
			left = mid + 1;
		}
		else if (weight > 1) {
			right = mid;
		}
		else if (weight == 1) {
			return mid;
		}
		else {
			if (p.pointAbovePlane(vertices[order[mid]].getCoord()) == 0) {
				left = mid + 1;
			}
			else {
				return mid;
			}
		}
	}
	return order.size();
}

//Getters for private variables.
std::vector<Point> OBJReader::getVertices() {
	return vertices;
}

Point OBJReader::getVertex(int i) {
	return vertices[i];
}

std::vector<Face> OBJReader::getFaces() {
	return faces;
}

std::map<int,int> OBJReader::getOrder() {
	return order;
}

std::pair<float, float> OBJReader::getMinMax() {
	return minMax;
}