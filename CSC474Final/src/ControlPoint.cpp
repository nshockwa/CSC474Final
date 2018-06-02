#include <stdio.h>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <string>
#include "glm/gtx/string_cast.hpp"


#include "ControlPoint.h"

using namespace std;
using namespace glm;

void ControlPoint::loadPoints(string filename) {

	file.open(filename);
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
	}

	// read file and store points
	float i, j, k;

	//while (file.peek() != EOF) {
	try {

		string temp;
		
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
		cout << "ERROR: File format must be: \n <text>" << endl;
	}


	cout << "printing out points" << endl;
	for (int i = 0; i < points.size(); i++) {
		cout << to_string(points[i]) << endl;
	}

}