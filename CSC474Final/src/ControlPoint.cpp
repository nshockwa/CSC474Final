#include <stdio.h>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <string>


#include "ControlPoint.h"

using namespace std;
using namespace glm;

void ControlPoint::loadPoints(string filename) {

	file.open(filename);
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
	}

	// read file and store points
	string str;
	getline(file, str, ' ');
	cout << "str to path name: " << str << endl;

}