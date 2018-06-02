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

	try {

		string line;
		while (file.peek() != EOF) {
			getline(file, line);							// get line from file

			const char * const_c_line = line.c_str();		// setup line to be parsed
			char * c_line = new char[line.length() + 1];
			strcpy(c_line, line.c_str());

			mat3 pt = mat3(1.0);							// store point here

			if (c_line[0] != '\n' && c_line[0] != '\0') {
					char * c = '\0';
					
					c = strtok(c_line, " \n");		// x	// X BASE
					pt[0].x = stof(string(c));
					c = strtok(NULL, " \n");		// y
					pt[0].y = stof(string(c));
					c = strtok(NULL, " \n");		// z
					pt[0].z = stof(string(c));

					c = strtok(NULL, " \n");		// x	// Y BASE
					pt[1].x = stof(string(c));
					c = strtok(NULL, " \n");		// y
					pt[1].y = stof(string(c));
					c = strtok(NULL, " \n");		// z
					pt[1].z = stof(string(c));

					c = strtok(NULL, " \n");		// x	// Z BASE
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
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
		return false;
	}

	file.close();
	return true;
}

// is it to open the file every time they wanna add points?
glm::mat3 ControlPoint::addPoint(vec3 pos, vec3 dir, vec3 up, string filename) {
	
	file.open(filename, ios::out | ios::app);	// ---------- open file | write | append
	if (!file.is_open()) {
		cout << "Warning: Could not open file - " << filename << endl;
	}

	mat3 pt = mat3(pos, dir, up);
	points.push_back(pt);

	// write mat to file
	file << endl;
	file << pos.x << " " << pos.y << " " << pos.z << " ";
	file << dir.x << " " << dir.y << " " << dir.z << " ";
	file << up.x << " " << up.y << " " << up.z << endl;

	file.close();

	return pt;
}

