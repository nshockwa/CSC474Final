#include <stdio.h>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <string>
#include "glm/gtx/string_cast.hpp"


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

	// read file and store points
	float i, j, k;

	//while (file.peek() != EOF) {
	try {

		int c = 0;
		string line; 
		mat3 pt = mat3(1.0);
		while ((c = file.peek()) != EOF) {
			if (c == '\n') getline(file, line);		// if new line, skip it;
			for (int i = 0; i < 3; i++) {
				vec3 xBase;
				getline(file, line, ' ');
				xBase.x = stof(line);		// x
				getline(file, line, ' ');
				xBase.y = stof(line);		// y
				getline(file, line, ' ');
				xBase.z = stof(line);		// z

				cout << "xBase: " << to_string(xBase) << endl;
				pt[i] = xBase;
			}
			points.push_back(pt);
		}

	}
	catch (exception e) {
		cout << "ERROR: File format must be: " << endl;
		cout << "/n " << endl;
		cout << "x1 y1 z1 x2 y2 z2 x3 y3 z3" << endl;
		cout << endl;
	}

	file.close();			// ------- close file


	/* TESTING */
	cout << "printing out points" << endl;
	for (int i = 0; i < points.size(); i++) {
		cout << to_string(points[i]) << endl;
	}
	return true;

}

bool ControlPoint::clearPoints(string filename) {
	cout << "[ControlPoints.cpp] Clearing points from file: " << filename << endl;
	file.open(filename, ios::out);	// ---------- open file
	file.close();
	return true;
}

