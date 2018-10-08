#pragma once
#include "include.h"

class Point {
public:
	string name;
	double height;

	Point(string name, double height) {
		this->name = name;
		this->height = height;
	}
};
