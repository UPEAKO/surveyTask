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
	double getRadianAngle(string aim1, string station, string aim2);
	double getRadianDirection(string station, string aim);
	string getThirdPoint(string point1, string point2);
	int getStationIndexByName(string name);
	bool getStartPoints(string& point1, string& point2);

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
	fstream f;
	cout << "请输入测角网初始数据路径（eg:D:/angle.txt),默认为当前路径下angle.txt:" << endl;
	string path = "";
	getline(cin, path);
	bool exist = Tool::fileExist(path);
	if (path == "") {
		path = "angle.txt";
		bool exist = Tool::fileExist(path);
		while (!exist) {
			cout << "路径无效！请重新输入：";
			getline(cin, path);
			exist = Tool::fileExist(path);
		}
		f.open(path);
	}
	else
	{
		bool exist = Tool::fileExist(path);
		while (!exist) {
			cout << "路径无效！请重新输入：";
			getline(cin, path);
			if (path == "") {
				path = "angle.txt";
				exist = Tool::fileExist(path);
			}
			else {
				exist = Tool::fileExist(path);
			}
		}
		f.open(path);
	}
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
	f.close();
}

//获取起算点坐标
bool angleAdjust::getStartPoints(string& point1, string& point2) {
	for (int k = 0; k < knownPointsNum; k++) {
		string knownPoint1 = points.at(k).name;
		for (int i = 0; i < directionValues.size(); i++) {
			if (directionValues.at(i).station == knownPoint1) {
				for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
					if (getPointByName(directionValues.at(i).aims.at(j).aim) < knownPointsNum) {
						point1 = knownPoint1;
						point2 = directionValues.at(i).aims.at(j).aim;
						return true;
					}
				}
			}
		}
	}
	return false;
}

//平差计算
void angleAdjust::adjust() {
	//近似坐标
	//标志位，所有近似坐标已计算出出
	int signForApproxiPoint = knownPointsNum;
	string point1, point2;
	if (!getStartPoints(point1, point2)) {
		cout << "无合适起算点！" << endl;
		return;
	}
	string tempPoint = getThirdPoint(point1, point2);
	while (signForApproxiPoint < allPointsNum) {
		if (tempPoint != "") {
			double angle1 = getRadianAngle(tempPoint, point1, point2);
			double angle2 = getRadianAngle(tempPoint, point2, point1);
			//返回4*PI暂时不考虑
			//余切
			double point1X = points.at(getPointByName(point1)).x;
			double point1Y = points.at(getPointByName(point1)).y;
			double point2X = points.at(getPointByName(point2)).x;
			double point2Y = points.at(getPointByName(point2)).y;
			//tempPoint在point1 -> point2的右侧
			bool right = getRadianDirection(point1,point2) < getRadianDirection(point1,tempPoint) ? true : false;
			if (right) {
				double tempPointX = (point1X / tan(angle2) + point2X / tan(angle1) - point2Y + point1Y) / (1 / tan(angle1) + 1 / tan(angle2));
				double tempPointY = (point1Y / tan(angle2) + point2Y / tan(angle1) + point2X - point1X) / (1 / tan(angle1) + 1 / tan(angle2));
				points.at(getPointByName(tempPoint)).x = tempPointX;
				points.at(getPointByName(tempPoint)).y = tempPointY;
				cout.precision(DBL_DECIMAL_DIG);
				signForApproxiPoint++;
			}
			else {
				double tempPointX = (point1X / tan(angle2) + point2X / tan(angle1) - point1Y + point2Y) / (1 / tan(angle1) + 1 / tan(angle2));
				double tempPointY = (point1Y / tan(angle2) + point2Y / tan(angle1) + point1X - point2X) / (1 / tan(angle1) + 1 / tan(angle2));
				points.at(getPointByName(tempPoint)).x = tempPointX;
				points.at(getPointByName(tempPoint)).y = tempPointY;
				signForApproxiPoint++;
			}
			//全体向前移动
			point1 = point2;
			point2 = tempPoint;
			tempPoint = getThirdPoint(point1, point2);
		}
		else {
			//是否有可能
		}
	}
	CMatrix<double> B(directionNum,(allPointsNum - knownPointsNum) * 2 + stationsNum);
	CMatrix<double> L(directionNum,1);
	int signForRow = 0;
	for (int i = 0; i < directionValues.size(); i++) {
		//史赖伯
		double Z0 = 0;
		int tempNum = directionValues.at(i).aims.size();
		double allDeltaAngle = 0;
		for (int k = 0; k < tempNum; k++) {
			int pointStation = getPointByName(directionValues.at(i).station);//j
			int pointAim = getPointByName(directionValues.at(i).aims.at(k).aim);//k
			double azimu = Tool::coordinateToAzimuthAngle(points.at(pointStation).x, points.at(pointStation).y, points.at(pointAim).x, points.at(pointAim).y);
			if (azimu < Tool::angleToRadian(directionValues.at(i).aims.at(k).direction)) {
				azimu += (2 * PI);
			}
			allDeltaAngle += (azimu - Tool::angleToRadian(directionValues.at(i).aims.at(k).direction));
		}
		Z0 = allDeltaAngle / tempNum;
		//系数及常数项
		for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
			//对每一个方向
			int pointStation = getPointByName(directionValues.at(i).station);//j
			int pointAim = getPointByName(directionValues.at(i).aims.at(j).aim);//k
			double deltaX = points.at(pointAim).x - points.at(pointStation).x;//Xjk
			double deltaY = points.at(pointAim).y - points.at(pointStation).y;//Yjk
			double S0Square = deltaX * deltaX + deltaY * deltaY;//Sjk
			//非已知存入
			//j
			if (pointStation >= knownPointsNum) {
				//y位置
				int locationY = (pointStation - knownPointsNum + 1) * 2 - 1;
				//double p11 = 180 * 60 * 60 / PI;
				double tempX = (deltaY / S0Square);
				double tempY = -(deltaX / S0Square);
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//k
			if (pointAim >= knownPointsNum) {
				//y位置
				int locationY = (pointAim - knownPointsNum + 1) * 2 - 1;
				//double p11 = 180 * 60 * 60 / PI;
				double tempX = -(deltaY / S0Square);
				double tempY = (deltaX / S0Square);
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//-z
			int colummnFor_z = (allPointsNum - knownPointsNum) * 2 + i;
			B(signForRow, colummnFor_z) = -1;
			//常数项L - (alpha - Z0)
			double azimu = Tool::coordinateToAzimuthAngle(points.at(pointStation).x, points.at(pointStation).y, points.at(pointAim).x, points.at(pointAim).y);
			double Ljk = getRadianDirection(points.at(pointStation).name, points.at(pointAim).name);
			if (azimu < Ljk)
				azimu += 2 * PI;
			double tempL = Ljk - (azimu - Z0);
			//cout << directionValues.at(i).station << "; " << directionValues.at(i).aims.at(j).aim << ": " << tempL << endl;
			L(signForRow, 0) = tempL;
			signForRow++;
		}

	}
	CMatrix<double> N_invertion = (B.transpose() * B).inversion();
	CMatrix<double> x = N_invertion * B.transpose() * L;
	CMatrix<double> v = B * x - L;
	
	//输出平差结果
	ofstream ofs;
	cout << "请设置测角网结果输出路径（eg:D:/angleResult.txt;默认在当前路径angleResult.txt）：" << endl;
	string resultPath = "";
	getline(cin, resultPath);
	if (resultPath == "")
		ofs.open("angleResult.txt");
	else
		ofs.open(resultPath);
	ofs.flags(ios::left);
	ofs.precision(DBL_DECIMAL_DIG);
	ofs << "测角网平差结果" << endl;
	ofs << endl;

	//后验中误差
	CMatrix<double> lateMatrix = v.transpose() * v;
	double lateError = sqrt(lateMatrix(0, 0) / (directionNum - ((allPointsNum - knownPointsNum) * 2 + stationsNum)));
	ofs << "后验单位权中误差: " << lateError << endl;
	ofs << endl;
	//点位中误差
	ofs << "点位中误差: " << endl;
	ofs << setw(5) << "点名" << setw(30) << "deltaX" << setw(30) << "deltaY" << setw(30) << "delta" << endl;
	for (int i = 0; i < allPointsNum - knownPointsNum; i++) {
		double sigmaX = lateError * sqrt(N_invertion(2 * i, 2 * i));
		double sigmaY = lateError * sqrt(N_invertion(2 * i + 1, 2 * i + 1));
		double sigma = lateError * sqrt(N_invertion(2 * i + 1, 2 * i + 1) + N_invertion(2 * i, 2 * i));
		string name = points.at(knownPointsNum + i).name;
		ofs << setw(5) << name << setw(30) << sigmaX << setw(30) << sigmaY << setw(30) << sigma << endl;
	}
	ofs << endl;

	ofs << "控制点成果： " << endl;
	ofs << setw(5) << "点名" << setw(20) << "X" << setw(20) << "Y" << endl;
	for (int i = 0; i < allPointsNum; i++) {
		if (i >= knownPointsNum) {
			points.at(i).x += x((i - knownPointsNum) * 2, 0);
			points.at(i).y += x((i - knownPointsNum) * 2 + 1, 0);
			ofs << setw(5) << points.at(i).name << setw(20) << points.at(i).x << setw(20) << points.at(i).y << endl;
		}
		else {
			ofs << setw(5) << points.at(i).name << setw(20) << points.at(i).x << setw(20) << points.at(i).y << endl;
		}
	}
	ofs.close();
}

//由两点获取三角形第三点未知名称,or 空串
//已有近似坐标的点sign = 0，未有为-1
string angleAdjust::getThirdPoint(string point1, string point2) {
	int point1Index = getStationIndexByName(point1);
	int point2Index = getStationIndexByName(point2);
	for (int i = 0; i < directionValues.at(point1Index).aims.size(); i++) {
		string tempPoint = directionValues.at(point1Index).aims.at(i).aim;
		if (tempPoint != point2) {
			for (int j = 0; j < directionValues.at(point2Index).aims.size(); j++) {
				if (tempPoint == directionValues.at(point2Index).aims.at(j).aim && points.at(getPointByName(tempPoint)).sign == -1)
					return tempPoint;
			}
		}
	}
	return "";
}

//由名称获取站点位置,否则-1
int angleAdjust::getStationIndexByName(string station) {
	for (int i = 0; i < directionNum; i++) {
		if (directionValues.at(i).station == station)
			return i;
	}
	return -1;
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

//返回弧度夹角,如果不存在,返回4*PI
double angleAdjust::getRadianAngle(string aim1, string station, string aim2) {
	double radianDirection1 = getRadianDirection(station, aim1);
	double radianDirection2 = getRadianDirection(station, aim2);
	if (radianDirection1 > 3 * PI || radianDirection2 > 3 * PI)
		return 4 * PI;
	if (radianDirection1 > radianDirection2)
		return radianDirection1 - radianDirection2;
	else
		return radianDirection2 - radianDirection1;
}

//返回弧度方向观测值，没有返回4*PI
double angleAdjust::getRadianDirection(string station, string aim) {
	for (int i = 0; i < directionValues.size(); i++) {
		if (station == directionValues.at(i).station) {
			for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
				if (aim == directionValues.at(i).aims.at(j).aim) {
					return Tool::angleToRadian(directionValues.at(i).aims.at(j).direction);
				}
			}
		}
	}
	return 4*PI;
}