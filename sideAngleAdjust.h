#pragma once
#include "directionValue.h"
#include "sideValue.h"
#include "PointV1.h"
#include "azimuthValue.h"
#include "Tool.h"

class sideAngleAdjust {
public:
	sideAngleAdjust();
	~sideAngleAdjust() {};
	void getAllValues();
	void adjustment();
	void getApproxiCoordinate(string station,string otherKnown,double lastDirection, double lastX, double lastY);
	double getRadianDirection(string station, string aim);
	bool isKnownPoint(string name);
	vector<string> getNextNames(string station, string otherKnown);
	int getPointByName(string name);
private:
	//边，方向，方位角
	int sideValueNum;
	vector<sideValue> sideValues;
	int directionNum;
	vector<directionValue> directionValues;
	int azimuthValueNum;
	vector<azimuthValue> azimuthValues;
	//已知点
	int knownPointsNum;
	int allPointsNum;
	vector<PointV1> points;
	//中误差
	double DirectMeanError;
	double DistanceFixedError, DistanceScaleError;
	double AzimuthMeanError;
	//方程个数
	int rowsOfB;
};

sideAngleAdjust::sideAngleAdjust() {
	getAllValues();
}

void sideAngleAdjust::getAllValues() {
	ifstream f;
	f.open("E:/cpp/data/sideAngle.txt");
	string eachLine;
	vector<string> eachLines;
	//line one
	getline(f, eachLine);
	eachLines = Tool::split(eachLine, ' ');
	allPointsNum = Tool::toInt(eachLines.at(0));
	knownPointsNum = Tool::toInt(eachLines.at(1));
	int stationsNum = Tool::toInt(eachLines.at(2));
	directionNum = Tool::toInt(eachLines.at(3));
	sideValueNum = Tool::toInt(eachLines.at(4));
	//B矩阵行
	rowsOfB += sideValueNum;
	azimuthValueNum = Tool::toInt(eachLines.at(5));
	//line two
	getline(f, eachLine);
	eachLines = Tool::split(eachLine, ' ');
	DirectMeanError = Tool::toDouble(eachLines.at(0));
	DistanceFixedError = Tool::toDouble(eachLines.at(1));
	DistanceScaleError = Tool::toDouble(eachLines.at(2));
	AzimuthMeanError = Tool::toDouble(eachLines.at(3));
	//已知点
	for (int i = 0; i < knownPointsNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		PointV1 PV(eachLines.at(0), Tool::toDouble(eachLines.at(1)), Tool::toDouble(eachLines.at(2)),0);
		points.push_back(PV);
	}

	getline(f, eachLine);//空行

	//方向值
	for (int i = 0; i < stationsNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		string station = eachLines.at(0);
		//添加未知点

		int nums = Tool::toInt(eachLines.at(1));
		//B矩阵行
		rowsOfB += (nums - 1);
		vector<eachDirectionFromStation> edfs;
		for (int j = 0; j < nums; j++) {
			getline(f, eachLine);
			eachLines = Tool::split(eachLine, ' ');
			edfs.push_back(eachDirectionFromStation(eachLines.at(0), Tool::toDouble(eachLines.at(1))));
		}
		directionValue dV(station,edfs);
		directionValues.push_back(dV);
	}

	getline(f, eachLine);//空行

	//边长
	for (int i = 0; i < sideValueNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		sideValue sV(eachLines.at(0), eachLines.at(1), Tool::toDouble(eachLines.at(2)));
		sideValues.push_back(sV);
	}

	getline(f, eachLine);//空行

	for (int i = 0; i < azimuthValueNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		azimuthValue aV(eachLines.at(0), eachLines.at(1), Tool::toDouble(eachLines.at(2)));
		azimuthValues.push_back(aV);
	}
}

//平差计算
void sideAngleAdjust::adjustment() {
	//两已知点线段开始计算近似坐标
	for (int i = 0; i < sideValueNum; i++) {
		//两端点已知
		if (isKnownPoint(sideValues.at(i).oneSide) && isKnownPoint(sideValues.at(i).anotherSide)) {
			getApproxiCoordinate(sideValues.at(i).oneSide, sideValues.at(i).anotherSide);
			break;
		}
	}
	//角度误差方程系数及常数项
	//系数阵
	CMatrix<double> B(rowsOfB, (allPointsNum - knownPointsNum) * 2);
	//常数阵
	CMatrix<double> L(rowsOfB, 1);
	//sign for rows ++
	int signForRow = 0;
	for (int i = 0; i < directionValues.size(); i++) {
		for (int j = 0; j < directionValues.at(i).aims.size() - 1; j++) {
			//对每一个夹角
			int point1 = getPointByName(directionValues.at(i).aims.at(j).aim);//h
			int pointStation = getPointByName(directionValues.at(i).station);//j
			int point2 = getPointByName(directionValues.at(i).aims.at(j + 1).aim);//k
			//涉及6各未知数，
			double deltaX1 = points.at(point1).x - points.at(pointStation).x;//Xjh
			double deltaY1 = points.at(point1).y - points.at(pointStation).y;//Yjh
			double deltaX2 = points.at(point2).x - points.at(pointStation).x;//Xjk
			double deltaY2 = points.at(point2).y - points.at(pointStation).y;//Yjk
			double S0Square1 = deltaX1 * deltaX1 + deltaY1 * deltaY1;//Sjh
			double S0Square2 = deltaX2 * deltaX2 + deltaY2 * deltaY2;//Sjk
			//非已知存入
			//j
			if (pointStation >= knownPointsNum) {
				//y位置
				int locationY = (pointStation - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = (deltaY2 / S0Square2 - deltaY1 / S0Square1) * p11;
				double tempY = (deltaX1 / S0Square1 - deltaX2 / S0Square2) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//h
			if (point1 >= knownPointsNum) {
				//y位置
				int locationY = (point1 - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = (deltaY1 / S0Square1) * p11;
				double tempY = -(deltaX1 / S0Square1) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//k
			if (point1 >= knownPointsNum) {
				//y位置
				int locationY = (point2 - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = -(deltaY2 / S0Square1) * p11;
				double tempY = (deltaX1 / S0Square1) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//常数项L - L0
			double tempL = Tool::angle2MinusAngle1(directionValues.at(i).aims.at(j + 1).direction, directionValues.at(i).aims.at(j).direction);
			//any problem?
			double tempL0 = Tool::angle2MinusAngle1(Tool::coordinateToAzimuthAngle(points.at(pointStation).x,points.at(pointStation).y,points.at(point2).x,points.at(point2).y),
				Tool::coordinateToAzimuthAngle(points.at(pointStation).x, points.at(pointStation).y, points.at(point1).x, points.at(point1).y));
			L(rowsOfB, 1) = tempL - tempL0;
			signForRow++;
		}
	}
	//边长误差方程系数及常数项
	for (int i = 0; i < sideValueNum; i++) {
		int j = getPointByName(sideValues.at(i).oneSide);
		int k = getPointByName(sideValues.at(i).anotherSide);
		double deltaXjk = points.at(k).x - points.at(j).x;
		double deltaYjk = points.at(k).y - points.at(j).y;
		double S0jk = sqrt(deltaXjk * deltaXjk + deltaYjk * deltaYjk);
		if (j >= knownPointsNum) {
			//Y位置
			int locationY = (j - knownPointsNum + 1) * 2 - 1;
			double tempX = -deltaXjk / S0jk;
			double tempY = -deltaYjk / S0jk;
			B(signForRow, locationY - 1) = tempX;
			B(signForRow, locationY) = tempY;
		}
		if (k >= knownPointsNum) {
			//Y位置
			int locationY = (k - knownPointsNum + 1) * 2 - 1;
			double tempX = deltaXjk / S0jk;
			double tempY = deltaYjk / S0jk;
			B(signForRow, locationY - 1) = tempX;
			B(signForRow, locationY) = tempY;
		}
		//常数项L - S0
		L(signForRow, 1) = sideValues.at(i).len - S0jk;
		signForRow++;
	}
	//权阵P
}

//如果不存在，添加值到最后
int sideAngleAdjust::getPointByName(string name) {
	for (int i = 0; i < points.size(); i++) {
		if (points.at(i).name == name)
			return i;
	}
	points.push_back(PointV1(name, 0, 0, -1));
	return points.size() - 1;
}

//获取未知点坐标近似值,递归计算;lastInfo:方位角，x,y
void sideAngleAdjust::getApproxiCoordinate(string station, string otherKnown,double lastDirection, double lastX, double lastY) {
	//如果没有下一个可计算点
	if (false)
		return;
	//oneSide始终为站
	vector<string> nextNames = getNextNames(station,otherKnown);
	for (int i = 0; i < nextNames.size(); i++) {
		double leftAngle = getLeftAngle(otherKnown, station, nextNames.at(i));
		double len = getLen(station, nextNames.at(i));
		double direction = lastDirection + leftAngle - PI;
		if (direction > 2 * PI)
			direction -= 2 * PI;
		double currentX = lastX + len * cos(direction);
		double currentY = lastY + len * sin(direction);
		//points.push_back(PointV1(nextNames.at(i), currentX,currentY,0));
		points.at(getPointByName(nextNames.at(i))).x = currentX;
		points.at(getPointByName(nextNames.at(i))).y = currentY;
		points.at(getPointByName(nextNames.at(i))).sign = 1;
		getApproxiCoordinate(nextNames.at(i), station, direction, currentX, currentY);
	}
}

vector<string> sideAngleAdjust::getNextNames(string station, string otherKnown) {

}

bool sideAngleAdjust::isKnownPoint(string name) {
	for (int i = 0; i < knownPointsNum; i++) {
		if (points.at(i).name == name)
			return true;
	}
	return false;
}

//获取方向值
double sideAngleAdjust::getRadianDirection(string station,string aim) {
	for (int i = 0; i < directionValueNum; i++) {
		if (station == directionValues.at(i).station) {
			for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
				if (aim == directionValues.at(i).aims.at(j).aim) {
					return Tool::angleToRadian(directionValues.at(i).aims.at(j).direction);
				}
			}
		}
		else {
			return -1;
		}
	}
	return -1;
}