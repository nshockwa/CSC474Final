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
	
	string strr;
	//getline(file, strr);
	cout << "str to path name: " << strr << endl;


	mat3 pt = mat3(1.0);
	//while (file.peek() != EOF) {
	try {
		while (getline(file, strr)) {
			for (int i = 0; i < 3; i++) {
				vec3 xBase;

				getline(file, strr, ' ');
				xBase.x = stof(strr);		// x
				getline(file, strr, ' ');
				xBase.y = stof(strr);		// y
				getline(file, strr, ' ');
				xBase.z = stof(strr);		// z
				cout << to_string(xBase) << endl;

				pt[i] = xBase;
			}
			points.push_back(pt);
		}
	}
	catch (exception e) {
		cout << "ERROR: File format must be: \n <text>" << endl;
	}



	for (int i = 0; i < points.size(); i++) {
		cout << to_string(points[i]) << endl;
	}

}