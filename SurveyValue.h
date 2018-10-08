#pragma once
#include "include.h"

class SurveyValue{
public:
	SurveyValue(string begin, string end, double eachHeight, double eachLength, double deltaHeight,double eachHeightAfterCorrect) {
		this->begin = begin;
		this->end = end;
		this->eachHeight = eachHeight;
		this->eachLength = eachLength;
		this->deltaHeight = deltaHeight;
	}

	string begin;
	string end;
	//某段高差值
	double eachHeight;
	//某段长度
	double eachLength;
	//高差改正数
	double deltaHeight;
	//改正后高差
	double eachHeightAfterCorrect;
};
