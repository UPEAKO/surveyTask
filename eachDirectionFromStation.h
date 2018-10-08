#pragma once
#include "include.h"
class eachDirectionFromStation {
public:
	eachDirectionFromStation(string aim, double direction) {
		this->aim = aim;
		this->direction = direction;
	}

	string aim;
	double direction;
};