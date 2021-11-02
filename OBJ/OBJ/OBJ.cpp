// OBJ.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "OBJReader.h"
#include "OBJSlicer.h"
#include "Plane.h"

int main()
{
	//Plane p1 = Plane(glm::vec3(-0.3, -0.2, -0.1), glm::vec3(0.3, -0.2, -0.1), glm::vec3(0, 0.3, 0.1));
	Plane p1 = Plane(glm::vec3(-0.2, 0.17, -0.3), glm::vec3(-0.2, 0.17, 0.3), glm::vec3(0.3, 0.17, 0));
	OBJReader o = OBJReader("C:\\CuraIO\\Input\\Bunny-LowPoly__Smooth.obj");
	OBJSlicer sl = OBJSlicer(&o, p1);
	sl.makePath(50,0,50);
	std::ofstream gcode("C:\\CuraIO\\Input\\gcodeTest.gcode");
	if (gcode.is_open()) {
		gcode << sl.getGCode().rdbuf();
	}
	std::cout << sl.getPathLength() << std::endl;
}
