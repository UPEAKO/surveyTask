#pragma once
#include "directionValue.h"
#include "PointV1.h"
#include "Tool.h"
#include "include.h"

class angleAdjust {
public:
	angleAdjust();
	void adjust();
private:
	int getPointByName(string name);

	int directionNum;
	vector<directionValue> directionValues;
	int stationsNum;
	int knownPointsNum;
	int allPointsNum;
	vector<PointV1> points;
	double meanError;
};

//构造获取已知数据，未知点初始化为0
angleAdjust::angleAdjust() {
	ifstream f;
	f.open("E:/cpp/data/angle.txt");
	string eachLine;
	vector<string> eachLines;
	//line one
	getline(f, eachLine);
	eachLines = Tool::split(eachLine, ' ');
	allPointsNum = Tool::toInt(eachLines.at(0));
	knownPointsNum = Tool::toInt(eachLines.at(1));
	stationsNum = Tool::toInt(eachLines.at(2));
	directionNum = Tool::toInt(eachLines.at(3));
	//line two
	getline(f, eachLine);
	eachLines = Tool::split(eachLine, ' ');
	meanError = Tool::toDouble(eachLines.at(0));
	//已知点
	for (int i = 0; i < knownPointsNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		PointV1 PV(eachLines.at(0), Tool::toDouble(eachLines.at(1)), Tool::toDouble(eachLines.at(2)), 0);
		points.push_back(PV);
	}

	getline(f, eachLine);//空行

	//方向值
	for (int i = 0; i < stationsNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		string station = eachLines.at(0);
		//添加未知点
		getPointByName(station);
		int nums = Tool::toInt(eachLines.at(1));
		vector<eachDirectionFromStation> edfs;
		for (int j = 0; j < nums; j++) {
			getline(f, eachLine);
			eachLines = Tool::split(eachLine, ' ');
			edfs.push_back(eachDirectionFromStation(eachLines.at(0), Tool::toDouble(eachLines.at(1))));
		}
		directionValue dV(station, edfs);
		directionValues.push_back(dV);
	}
}

//平差计算
void angleAdjust::adjust() {
	//近似坐标

}

//如果不存在，添加值到最后
int angleAdjust::getPointByName(string name) {
	for (int i = 0; i < points.size(); i++) {
		if (points.at(i).name == name)
			return i;
	}
	points.push_back(PointV1(name, 0, 0, -1));
	return points.size() - 1;
}