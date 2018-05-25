#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

#include "glm/gtc/quaternion.hpp"

/***************************************/

#pragma  once
#ifndef ControlPoint_h
#define ControlPoint_h

class ControlPoint {
public:

	vector<mat3> points;

	ifstream file;

	ControlPoint() {}
	~ControlPoint() {}

	void loadPoints(string filename);



}

#endif /* CountrolPoint>h */