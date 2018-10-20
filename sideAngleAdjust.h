#pragma once
#include "directionValue.h"
#include "sideValue.h"
#include "PointV1.h"
#include "azimuthValue.h"
#include "Tool.h"
#include "CMatrix.h"
#include "include.h"

/*
����ͬ�����
����L������нϴ�ֵ
��ȡ���Ƿ�Խ��Ʒ�λ�Ǽ�2*PIδʹ�ã���λ�� < ���������
�������˷�λ�� < �����
������ÿһ����վ�ĳ�ʼ�߼�Ϊ����Ǳߣ�����ֵ���࣬����Դ�С����ȷ���������п���L����2*PI��
���½ϴ�ƫ��
���ո�������������ȷ�����ֵ��5cm֮��
���Ѹ���λ�ǲ���1",�ǧ�ף��޷�ƽ���������
*/
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
	int sideValueNum;//9
	vector<sideValue> sideValues;
	int directionNum;//20
	vector<directionValue> directionValues;
	int azimuthValueNum;//1
	vector<azimuthValue> azimuthValues;
	//��֪��
	int knownPointsNum;
	int allPointsNum;
	vector<PointV1> points;
	//�����
	double DirectMeanError;
	double DistanceFixedError, DistanceScaleError;
	double AzimuthMeanError;
	//��վ��
	int stationsNum;
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
	stationsNum = Tool::toInt(eachLines.at(2));
	directionNum = Tool::toInt(eachLines.at(3));
	sideValueNum = Tool::toInt(eachLines.at(4));
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
		int nums = Tool::toInt(eachLines.at(1));
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
	//ռλ:�˴���Ӳ����������ӵ���֪�㺯��
	double x1 = points.at(getPointByName("A")).x;
	double y1 = points.at(getPointByName("A")).y;
	points.at(getPointByName("A")).sign = 1;
	double x2 = points.at(getPointByName("B")).x;
	double y2 = points.at(getPointByName("B")).y;
	points.at(getPointByName("B")).sign = 1;
	double azimuth = Tool::coordinateToAzimuthAngle(x1, y1, x2, y2);
	getApproxiCoordinate("B", "A", azimuth, x2, y2);
	//30 21
	CMatrix<double> B(sideValueNum + directionNum + azimuthValueNum, (allPointsNum - knownPointsNum) * 2 + stationsNum);
	CMatrix<double> L(sideValueNum + directionNum + azimuthValueNum, 1);
	//����ֵ���̼�������
	int signForRow = 0;
	for (int i = 0; i < directionValues.size(); i++) {
		//ʷ����
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
		for (int j = 0; j < directionValues.at(i).aims.size(); j++) {
			//��ÿһ������
			int pointStation = getPointByName(directionValues.at(i).station);//j
			int pointAim = getPointByName(directionValues.at(i).aims.at(j).aim);//k
			double deltaX = points.at(pointAim).x - points.at(pointStation).x;//Xjk
			double deltaY = points.at(pointAim).y - points.at(pointStation).y;//Yjk
			double S0Square = deltaX * deltaX + deltaY * deltaY;//Sjk
			//����֪����
			//j
			if (pointStation >= knownPointsNum) {
				//yλ��
				int locationY = (pointStation - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = (deltaY / S0Square) * p11;
				double tempY = -(deltaX / S0Square) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//k
			if (pointAim >= knownPointsNum) {
				//yλ��
				int locationY = (pointAim - knownPointsNum + 1) * 2 - 1;
				double p11 = 180 * 60 * 60 / PI;
				double tempX = -(deltaY / S0Square) * p11;
				double tempY = (deltaX / S0Square) * p11;
				B(signForRow, locationY - 1) = tempX;
				B(signForRow, locationY) = tempY;
			}
			//-z
			int colummnFor_z = (allPointsNum - knownPointsNum) * 2 + i;
			B(signForRow, colummnFor_z) = -1;
			//������L - (alpha - Z0)
			double azimu = Tool::coordinateToAzimuthAngle(points.at(pointStation).x, points.at(pointStation).y, points.at(pointAim).x, points.at(pointAim).y);
			double Ljk = getRadianDirection(points.at(pointStation).name, points.at(pointAim).name);
			if (azimu < Ljk)
				azimu += (2 * PI);
			double tempL = Ljk - (azimu - Z0);
			L(signForRow, 0) = tempL;
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
		L(signForRow, 0) = sideValues.at(i).len - S0jk;
		signForRow++;
	}
	//��λ�����̼�������
	for (int i = 0; i < azimuthValueNum; i++) {
		//��ÿһ������
		int pointStation = getPointByName(azimuthValues.at(i).start);//j
		int pointAim = getPointByName(azimuthValues.at(i).end);//k
		double deltaX = points.at(pointAim).x - points.at(pointStation).x;//Xjk
		double deltaY = points.at(pointAim).y - points.at(pointStation).y;//Yjk
		double S0Square = deltaX * deltaX + deltaY * deltaY;//Sjk
		//����֪����
			//j
		if (pointStation >= knownPointsNum) {
			//yλ��
			int locationY = (pointStation - knownPointsNum + 1) * 2 - 1;
			double p11 = 180 * 60 * 60 / PI;
			double tempX = (deltaY / S0Square) * p11;
			double tempY = -(deltaX / S0Square) * p11;
			B(signForRow, locationY - 1) = tempX;
			B(signForRow, locationY) = tempY;
		}
		//k
		if (pointAim >= knownPointsNum) {
			//yλ��
			int locationY = (pointAim - knownPointsNum + 1) * 2 - 1;
			double p11 = 180 * 60 * 60 / PI;
			double tempX = -(deltaY / S0Square) * p11;
			double tempY = (deltaX / S0Square) * p11;
			B(signForRow, locationY - 1) = tempX;
			B(signForRow, locationY) = tempY;
		}
		//������
		double azimu = Tool::coordinateToAzimuthAngle(points.at(pointStation).x, points.at(pointStation).y, points.at(pointAim).x, points.at(pointAim).y);
		double tempY = Tool::angleToRadian(azimuthValues.at(i).azimuthVal) - azimu;
		L(signForRow, 0) = tempY;
		signForRow++;
	}
	//Ȩ��P 21*21
	int PRC = sideValueNum + directionNum + azimuthValueNum;
	CMatrix<double> P(PRC, PRC);
	//�����
	int t = 0;
	for (; t < directionNum; t++)
		P(t, t) = 1;
	//��
	for (; t < sideValueNum + directionNum; t++) {
		int tempForSide1 = t - directionNum;
		double tempP = (DirectMeanError * DirectMeanError)
			/ ((DistanceFixedError + DistanceScaleError * sideValues.at(tempForSide1).len)
				* (DistanceFixedError + DistanceScaleError * sideValues.at(tempForSide1).len));
		P(t, t) = tempP;
	}
	//��λ��
	for (; t < PRC; t++)
		P(t, t) = (DirectMeanError * DirectMeanError) / (AzimuthMeanError * AzimuthMeanError);
	CMatrix<double> x = (B.transpose() * P * B).inversion() * B.transpose() * P * L;
	cout << "������" << endl;
	for (int i = knownPointsNum; i < allPointsNum; i++) {
		points.at(i).x += x((i - 3) * 2, 0);
		points.at(i).y += x((i - 3) * 2 + 1, 0);
		cout << points.at(i).name << ": " << points.at(i).x << " , " << points.at(i).y << endl;
	}
	CMatrix<double> v = B * x - L;
	//��������
	CMatrix<double> lateMatrix = v.transpose() * P * v;
	double lateError = sqrt(lateMatrix(0, 0) / ((sideValueNum + directionNum + azimuthValueNum) - ((allPointsNum - knownPointsNum) * 2 + stationsNum)));
	cout << "�������" << lateError << endl;
}

//��ȡδ֪���������ֵ,�ݹ����;
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
		direction = lastDirection + leftAngle - PI;
		if (direction > 2 * PI)
			direction -= 2 * PI;
		double currentX = 0, currentY = 0;
		if (isKnownPoint(nextNames.at(i))) {
			currentX = points.at(getPointByName(nextNames.at(i))).x;
			currentY = points.at(getPointByName(nextNames.at(i))).y;

			getApproxiCoordinate(nextNames.at(i), station, direction, currentX, currentY);
		}
		else {
			currentX = lastX + len * cos(direction);
			currentY = lastY + len * sin(direction);
			points.at(getPointByName(nextNames.at(i))).x = currentX;
			points.at(getPointByName(nextNames.at(i))).y = currentY;
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
			return tempAzimuth;
		}
	}
	//�޴˷�λ��
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
