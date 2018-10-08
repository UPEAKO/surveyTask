#pragma once
#include "include.h"
class azimuthValue {
public:
	azimuthValue(string start, string end, double azimuthVal) {
		this->start = start;
		this->end = end;
		this->azimuthVal = azimuthVal;
	}

	string start;
	string end;
	double azimuthVal;
};
