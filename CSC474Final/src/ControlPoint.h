#pragma  once
#ifndef ControlPoint_h
#define ControlPoint_h

#include <stdio.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <vector>
//#include <string>


/***************************************/

class ControlPoint {
public:

	std::vector<glm::mat3> points;

	std::ifstream file;

	ControlPoint() {}
	~ControlPoint() {}

	void loadPoints(std::string filename);



};

#endif /* CountrolPoint_h */