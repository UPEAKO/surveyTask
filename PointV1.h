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
	int sign;//-1未知，0已知, 1近似值/已在递归计算近似值时选中；
};