#pragma once
#include "include.h"

class PointV1 {
public:
	PointV1(string name, double x, double y,int sign) {
		this->name = name;
		this->x = x;
		this->y = y;
		this->sign = sign;
	}
	string name;
	double x;
	double y;
	int sign;//-1δ֪��0��֪, 1����ֵ/���ڵݹ�������ֵʱѡ�У�
};