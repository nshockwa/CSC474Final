#include <stdio.h>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include <glm/gtc/matrix_transform.hpp>


#include "ControlPoint.h"

using namespace std;
using namespace glm;

bool ControlPoint::loadPoints(string filename) {

	cout << "[ControlPoints.cpp] Loading points from file: " << filename << endl;

	file.open(filename, ios::in);	// ---------- open file
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
		return false;
	}

	try {

		string line;
		while (file.peek() != EOF) {
			getline(file, line);							// get line from file

			const char * const_c_line = line.c_str();		// setup line to be parsed
			char * c_line = new char[line.length() + 1];
			strcpy(c_line, line.c_str());

			mat3 pt = mat3(1.0);							// store point here

			if (c_line[0] != '\n' && c_line[0] != '\0') {
					char * c = "\0";

					c = strtok(c_line, " \n");		// x	// X BASE - pos
					pt[0].x = stof(string(c));
					c = strtok(NULL, " \n");		// y
					pt[0].y = stof(string(c));
					c = strtok(NULL, " \n");		// z
					pt[0].z = stof(string(c));

					c = strtok(NULL, " \n");		// x	// Y BASE - up
					pt[1].x = stof(string(c));
					c = strtok(NULL, " \n");		// y
					pt[1].y = stof(string(c));
					c = strtok(NULL, " \n");		// z
					pt[1].z = stof(string(c));

					c = strtok(NULL, " \n");		// x	// Z BASE - dir
					pt[2].x = stof(string(c));
					c = strtok(NULL, " \n");		// y
					pt[2].y = stof(string(c));
					c = strtok(NULL, " \n");		// z
					pt[2].z = stof(string(c));

					points.push_back(pt);			// add point to vector
				}

			delete[] c_line;								// delete temp string buffer
		}

	}
	catch (exception e) {
		cout << "ERROR: File format must be: " << endl;
		cout << "/n " << endl;
		cout << "x1 y1 z1 x2 y2 z2 x3 y3 z3" << endl;
		cout << endl;
	}


	// build model matrix for each point
	buildModelMat();

	file.close();			// ------- close file

	return true;

}

bool ControlPoint::clearPoints(string filename) {
	cout << "[ControlPoints.cpp] Clearing points from file: " << filename << endl;

	file.open(filename, ios::out);	// ---------- open file
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
		return false;
	}

	file.close();
	return true;
}

// is it to open the file every time they wanna add points?
glm::mat3 ControlPoint::addPoint(vec3 pos, vec3 up, vec3 dir, string filename) {

	file.open(filename, ios::out | ios::app);	// ---------- open file | write | append
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
	}

	pos *= -1.0f; //up *= -1.0f; dir *= -1.0f;				// control point is inverse of camera pos
	mat3 pt = mat3(pos, up, dir);
	points.push_back(pt);

	// write mat to file
	file << endl;
	file << pos.x << " " << pos.y << " " << pos.z << " ";		// pos
	file << up.x << " " << up.y << " " << up.z << " ";			// up
	file << dir.x << " " << dir.y << " " << dir.z << endl;		// dir

	// build model matrix for each point
	buildModelMat();

	file.close();

	return pt;
}

// Fill array of model matrices
void ControlPoint::buildModelMat() {

	for (int i = 0; i < points.size(); i++) {

		// get rotation
		mat4 rotM = mat4(1.0);
		vec3 ey = points[i][1];
		vec3 ez = points[i][2];
		vec3 ex = cross(ey, ez);
		rotM[0][0] = ex.x;	rotM[1][0] = ey.x;	rotM[2][0] = ez.x;	rotM[3][0] = 0;
		rotM[0][1] = ex.y;	rotM[1][1] = ey.y;	rotM[2][1] = ez.y;	rotM[3][1] = 0;
		rotM[0][2] = ex.z;	rotM[1][2] = ey.z;	rotM[2][2] = ez.z;	rotM[3][2] = 0;
		rotM[0][3] = 0;		rotM[1][3] = 0;		rotM[2][3] = 0;		rotM[3][3] = 1.0f;
		quat q = quat(rotM);
		q = normalize(q);
		rotM = mat4(q);
		mat4 transCP = translate(mat4(1.0), points[i][0]);	// translate

		mat4 M = transCP * rotM;								// model mat
		modelMats.push_back(M);
	}
}

// don't really need this method?
glm::mat4 ControlPoint::getModelMat(int idx) {

	if (modelMats.size() <= 0) {
		cout << "ControlPoint::Error: Model matrices not built!" << endl;
		return glm::mat4(0.0);
	}
	if (modelMats.size() <= idx) {
		cout << "ControlPoint::Error: index out of bound." << endl;
		return glm::mat4(0.0);
	}

	return modelMats[idx];
}

int ControlPoint::getSize() {
	return points.size();
}

glm::mat3 ControlPoint::goToLastPoint() {
	if (points.size() == 0) {
		cout << "ControlPoint::goToLastPointError: index out of bound." <<endl;
		return mat3(0.0);
	}
	return points[points.size()-1];
}
