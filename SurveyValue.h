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
	//ĳ�θ߲�ֵ
	double eachHeight;
	//ĳ�γ���
	double eachLength;
	//�߲������
	double deltaHeight;
	//������߲�
	double eachHeightAfterCorrect;
};
