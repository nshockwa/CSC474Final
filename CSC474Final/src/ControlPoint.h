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
	std::vector<glm::mat4> modelMats;
	std::fstream file;

	ControlPoint() {}
	~ControlPoint() {}

	bool loadPoints(std::string filename);		// return false if file can't load
	bool clearPoints(std::string filename);
	glm::mat3 addPoint(glm::vec3 pos, glm::vec3 dir, glm::vec3 up, std::string filename);
	void buildModelMat();
	glm::mat4 getModelMat(int idx);
	int getSize();
	glm::mat3 goToLastPoint();


};

#endif /* CountrolPoint_h */
