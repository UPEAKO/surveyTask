#pragma once
#include "include.h"
#include "eachDirectionFromStation.h"
class directionValue {
public:
	directionValue(string station, vector<eachDirectionFromStation> aims) {
		this->station = station;
		this->aims = aims;
	}

	string station;
	vector<eachDirectionFromStation> aims;
};
