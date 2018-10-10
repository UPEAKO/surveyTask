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
	double getLeftAngle(string otherKnown, string station, string nextName);
	double getLen(string station, string nextName);
	int getPointByName(string name);
	double getKnownAzimuth(string station, string aim);
private:
	//�ߣ����򣬷�λ��
	int sideValueNum;
	vector<sideValue> sideValues;
	int directionNum;//20
	vector<directionValue> directionValues;
	int azimuthValueNum;
	vector<azimuthValue> azimuthValues;
	//��֪��
	int knownPointsNum;
	int allPointsNum;
	vector<PointV1> points;
	//�����
	double DirectMeanError;
	double DistanceFixedError, DistanceScaleError;
	double AzimuthMeanError;
	//���̸���
	int rowsOfB;
};

sideAngleAdjust::sideAngleAdjust() {
	getAllValues();
}

//��������
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
	//B������
	rowsOfB += sideValueNum;
	azimuthValueNum = Tool::toInt(eachLines.at(5));
	//line two
	getline(f, eachLine);
	eachLines = Tool::split(eachLine, ' ');
	DirectMeanError = Tool::toDouble(eachLines.at(0));
	DistanceFixedError = Tool::toDouble(eachLines.at(1));
	DistanceScaleError = Tool::toDouble(eachLines.at(2));
	AzimuthMeanError = Tool::toDouble(eachLines.at(3));
	//��֪��
	for (int i = 0; i < knownPointsNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		PointV1 PV(eachLines.at(0), Tool::toDouble(eachLines.at(1)), Tool::toDouble(eachLines.at(2)),0);
		points.push_back(PV);
	}

	getline(f, eachLine);//����

	//����ֵ
	for (int i = 0; i < stationsNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		string station = eachLines.at(0);
		//���δ֪��

		int nums = Tool::toInt(eachLines.at(1));
		//B������
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

	getline(f, eachLine);//����

	//�߳�
	for (int i = 0; i < sideValueNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		sideValue sV(eachLines.at(0), eachLines.at(1), Tool::toDouble(eachLines.at(2)));
		sideValues.push_back(sV);
	}

	getline(f, eachLine);//����

	for (int i = 0; i < azimuthValueNum; i++) {
		getline(f, eachLine);
		eachLines = Tool::split(eachLine, ' ');
		azimuthValue aV(eachLines.at(0), eachLines.at(1), Tool::toDouble(eachLines.at(2)));
		azimuthValues.push_back(aV);
	}
	f.close();
}

//ƽ�����
void sideAngleAdjust::adjustment() {
	//����֪���߶ο�ʼ�����������
	/*
	for (int i = 0; i < sideValueNum; i++) {
		//���˵���֪
		if (isKnownPoint(sideValues.at(i).oneSide) && isKnownPoint(sideValues.at(i).anotherSide)) {
			cout << "here1" << endl;
			//string station,string otherKnown,double lastDirection, double lastX, double lastY
			double x1 = points.at(getPointByName(sideValues.at(i).anotherSide)).x;
			double y1 = points.at(getPointByName(sideValues.at(i).anotherSide)).y;
			double x2 = points.at(getPointByName(sideValues.at(i).oneSide)).x;
			double y2 = points.at(getPointByName(sideValues.at(i).oneSide)).y;
			double azimuth = Tool::coordinateToAzimuthAngle(x1, y1, x2, y2);
			getApproxiCoordinate(sideValues.at(i).oneSide, sideValues.at(i).anotherSide,azimuth,x2,y2);
			break;
		}
	}
	*/
	double x1 = points.at(getPointByName("A")).x;
	double y1 = points.at(getPointByName("A")).y;
	points.at(getPointByName("A")).sign = 1;
	double x2 = points.at(getPointByName("B")).x;
	double y2 = points.at(getPointByName("B")).y;
	points.at(getPointByName("B")).sign = 1;
	double azimuth = Tool::coordinateToAzimuthAngle(x1, y1, x2, y2);
	getApproxiCoordinate("B", "A", azimuth, x2, y2);
	//�Ƕ�����ϵ����������
	CMatrix<double> B(rowsOfB, (allPointsNum - knownPointsNum) * 2);
	CMatrix<double> L(rowsOfB, 1);
	//sign for rows ++
	int signForRow = 0;
	for (int i = 0; i < directionValues.size(); i++) {
		for (int j = 0; j < directionValues.at(i).aims.size() - 1; j++) {
			//��ÿһ���н�
			int point1 = getPointByName(directionValues.at(i).aims.at(j).aim);//h
			int pointStation = getPointByName(directionValues.at(i).station);//j
			int point2 = getPointByName(directionValues.at(i).aims.at(j + 1).aim);//k
			//�漰6��δ֪����
			double deltaX1 = points.at(point1).x - points.at(pointStation).x;//Xjh
			double deltaY1 = points.at(point1).y - points.at(pointStation).y;//Yjh
			double deltaX2 = points.at(point2).x - points.at(pointStation).x;//Xjk
			double deltaY2 = points.at(point2).y - points.at(pointStation).y;//Yjk
			double S0Square1 = deltaX1 * deltaX1 + deltaY1 * deltaY1;//Sjh
			double S0Square2 = deltaX2 * deltaX2 + deltaY2 * deltaY2;//Sjk
			//����֪����
			//j
			if (pointStation >= knownPointsNum) {
				//yλ��
				int locationY = (pointStation - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = (deltaY2 / S0Square2 - deltaY1 / S0Square1) * p11;
				double tempY = (deltaX1 / S0Square1 - deltaX2 / S0Square2) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//h
			if (point1 >= knownPointsNum) {
				//yλ��
				int locationY = (point1 - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = (deltaY1 / S0Square1) * p11;
				double tempY = -(deltaX1 / S0Square1) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//k
			if (point2 >= knownPointsNum) {
				//yλ��
				int locationY = (point2 - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = -(deltaY2 / S0Square2) * p11;
				double tempY = (deltaX2 / S0Square2) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//������L - L0
			double tempL = Tool::angle2MinusAngle1(directionValues.at(i).aims.at(j + 1).direction, directionValues.at(i).aims.at(j).direction);
			//any problem?
			double tempL0 = Tool::angle2MinusAngle1(Tool::coordinateToAzimuthAngle(points.at(pointStation).x,points.at(pointStation).y,points.at(point2).x,points.at(point2).y),
				Tool::coordinateToAzimuthAngle(points.at(pointStation).x, points.at(pointStation).y, points.at(point1).x, points.at(point1).y));
			L(rowsOfB, 1) = tempL - tempL0;
			signForRow++;
		}
	}
	//�߳�����ϵ����������
	for (int i = 0; i < sideValueNum; i++) {
		int j = getPointByName(sideValues.at(i).oneSide);
		int k = getPointByName(sideValues.at(i).anotherSide);
		double deltaXjk = points.at(k).x - points.at(j).x;
		double deltaYjk = points.at(k).y - points.at(j).y;
		double S0jk = sqrt(deltaXjk * deltaXjk + deltaYjk * deltaYjk);
		//cout << i << ": " << S0jk << endl;
		if (j >= knownPointsNum) {
			//Yλ��
			int locationY = (j - knownPointsNum + 1) * 2 - 1;
			double tempX = -deltaXjk / S0jk;
			double tempY = -deltaYjk / S0jk;
			B(signForRow, locationY - 1) = tempX;
			B(signForRow, locationY) = tempY;
		}
		if (k >= knownPointsNum) {
			//Yλ��
			int locationY = (k - knownPointsNum + 1) * 2 - 1;
			double tempX = deltaXjk / S0jk;
			double tempY = deltaYjk / S0jk;
			B(signForRow, locationY - 1) = tempX;
			B(signForRow, locationY) = tempY;
		}
		//������L - S0
		L(signForRow, 1) = sideValues.at(i).len - S0jk;
		signForRow++;
	}
	cout << B;
	cout << L;
	cout << "here" << endl;
	//Ȩ��P

}

//��ȡδ֪���������ֵ,�ݹ����;lastInfo:��λ�ǣ�x,y
void sideAngleAdjust::getApproxiCoordinate(string station, string otherKnown,double lastDirection, double lastX, double lastY) {
	//���û����һ���ɼ����
	vector<string> nextNames = getNextNames(station, otherKnown);
	if (nextNames.size() == 0)
		return;
	//oneSideʼ��Ϊվ
	for (int i = 0; i < nextNames.size(); i++) {
		//����
		double direction = 0;
		double leftAngle = getLeftAngle(otherKnown, station, nextNames.at(i));
		double len = getLen(station, nextNames.at(i));
		//�Ѹ���λ�ǲ���1",�ǧ��
		//�����֪��λ��
		/*
		if ((direction = getKnownAzimuth(station, nextNames.at(i)) < 3 * PI)) {

		}
		else {
			direction = lastDirection + leftAngle - PI;
			if (direction > 2 * PI)
				direction -= 2 * PI;
		}*/
		//*
		direction = lastDirection + leftAngle - PI;
		if (direction > 2 * PI)
			direction -= 2 * PI;
		if (nextNames.at(i) == "2")
			cout << "�棺" << Tool::radianToAngle(direction) << endl;
			//*/
		double currentX = 0, currentY = 0;
		if (isKnownPoint(nextNames.at(i))) {
			currentX = points.at(getPointByName(nextNames.at(i))).x;
			currentY = points.at(getPointByName(nextNames.at(i))).y;
			getApproxiCoordinate(nextNames.at(i), station, direction, currentX, currentY);
		}
		else {
			currentX = lastX + len * cos(direction);
			currentY = lastY + len * sin(direction);
			//points.push_back(PointV1(nextNames.at(i), currentX,currentY,0));
			points.at(getPointByName(nextNames.at(i))).x = currentX;
			points.at(getPointByName(nextNames.at(i))).y = currentY;
			cout.precision(DBL_DECIMAL_DIG);
			cout << nextNames.at(i) << ": " << currentX << " , " << currentY << endl;
			points.at(getPointByName(nextNames.at(i))).sign = 1;
			getApproxiCoordinate(nextNames.at(i), station, direction, currentX, currentY);
		}
	}
}

//���ػ���ֵ�Ĳ�����λ��,����ֵ����4*PI�޴˷�λ��
double sideAngleAdjust::getKnownAzimuth(string station, string aim) {
	for (int i = 0; i < azimuthValueNum; i++) {
		//ƥ��˳��λ��
		if (azimuthValues.at(i).start == station && azimuthValues.at(i).end == aim) {
			return Tool::angleToRadian(azimuthValues.at(i).azimuthVal);
		}
		//����
		if (azimuthValues.at(i).start == aim && azimuthValues.at(i).end == station) {
			double tempAzimuth = Tool::angleToRadian(azimuthValues.at(i).azimuthVal) - PI;
			if (tempAzimuth < 0)
				tempAzimuth += 2 * PI;
			if (aim == "2")
				cout << "�棺" << Tool::radianToAngle(tempAzimuth) << endl;
			return tempAzimuth;
		}
	}
	//�޵ķ�λ��
	return 4 * PI;
}

//��ȡ���ֵ������
double sideAngleAdjust::getLeftAngle(string otherKnown, string station, string nextName) {
	//otherKnown->station->nextNameΪǰ������
	double radianDirection1 = getRadianDirection(station, otherKnown);
	double radianDirection2 = getRadianDirection(station, nextName);
	double tempRadian = radianDirection1 - radianDirection2;
	//�ҽ�
	if (tempRadian > 1E-9)
		return 2 * PI - tempRadian;
	else
		return -tempRadian;
}

//��ȡ����
double sideAngleAdjust::getLen(string station, string nextName) {
	for (int i = 0; i < sideValueNum; i++) {
		if ((station == sideValues.at(i).oneSide && nextName == sideValues.at(i).anotherSide)
			|| (station == sideValues.at(i).anotherSide && nextName == sideValues.at(i).oneSide))
			return sideValues.at(i).len;
	}
	return -1;
}

//�ݹ����һЩ��
vector<string> sideAngleAdjust::getNextNames(string station, string otherKnown) {
	vector<string> result;
	//�ڷ���۲�ֵ�в���
	for (int i = 0; i < directionValues.size(); i++) {
		if (station == directionValues.at(i).station) {
			for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
				if (points.at(getPointByName(directionValues.at(i).aims.at(j).aim)).sign != 1)
					result.push_back(directionValues.at(i).aims.at(j).aim);
			}
		}
	}
	return result;
}

bool sideAngleAdjust::isKnownPoint(string name) {
	for (int i = 0; i < knownPointsNum; i++) {
		if (points.at(i).name == name)
			return true;
	}
	return false;
}

//��ȡ����ֵ
double sideAngleAdjust::getRadianDirection(string station,string aim) {
	for (int i = 0; i < directionValues.size(); i++) {
		if (station == directionValues.at(i).station) {
			for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
				if (aim == directionValues.at(i).aims.at(j).aim) {
					return Tool::angleToRadian(directionValues.at(i).aims.at(j).direction);
				}
			}
		}
	}
	return -1;
}

//��������ڣ����ֵ�����
int sideAngleAdjust::getPointByName(string name) {
	for (int i = 0; i < points.size(); i++) {
		if (points.at(i).name == name)
			return i;
	}
	points.push_back(PointV1(name, 0, 0, -1));
	return points.size() - 1;
}
