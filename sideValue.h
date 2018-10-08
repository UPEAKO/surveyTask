#pragma once
#include "include.h"

class sideValue{
public:
	sideValue(string oneSide, string anotherSide, double len) {
		this->oneSide = oneSide;
		this->anotherSide = anotherSide;
		this->len = len;
	}

	string oneSide;
	string anotherSide;
	double len;
};
