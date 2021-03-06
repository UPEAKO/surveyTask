#include "CMatrix.h"
#include "Tool.h"
#include "LevelApproximation.h"
#include "LevelPrecision.h"
#include "include.h"
#include "sideAngleAdjust.h"
#include "angleAdjust.h"

int main() {
	/*
	CMatrix<double> A("E:/cpp/data/A.txt");
	CMatrix<double> B("E:/cpp/data/B.txt");
	cout << "����A:" << endl;
	cout << A;
	cout << "����B:" << endl;
	cout << B;
	cout << "����A + B:" << endl;
	cout << A + B;
	cout << "����A - B:" << endl;
	cout << A - B;
	cout << "����A * B:" << endl;
	cout << A * B;
	cout << "����B��ת��:" << endl;
	cout << B.transpose();
	cout << "����A������:" << endl;
	cout << A.inversion();
	cout << "90.0125ת���ȣ�" << endl;
	cout << Tool::angleToRadian(90.0125) << endl;
	cout << "ת�ؽǶ�" << endl;
	cout << Tool::radianToAngle(Tool::angleToRadian(90.0125)) << endl;
	cout << "�ڶ����޽�ƽ���߷�λ��:" << endl;
	cout << Tool::radianToAngle(Tool::coordinateToAzimuthAngle(-25.5, 25.5, -51.0, 51)) << endl;
	*/
	LevelApproximation levelA;
	levelA.Calculation();
	LevelPrecision levelP;
	levelP.calculation();
	sideAngleAdjust sAA;
	sAA.adjustment();
	angleAdjust AA;
    AA.adjust();
	return 0;
}