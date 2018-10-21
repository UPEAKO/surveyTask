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
	cout << "矩阵A:" << endl;
	cout << A;
	cout << "矩阵B:" << endl;
	cout << B;
	cout << "矩阵A + B:" << endl;
	cout << A + B;
	cout << "矩阵A - B:" << endl;
	cout << A - B;
	cout << "矩阵A * B:" << endl;
	cout << A * B;
	cout << "矩阵B的转置:" << endl;
	cout << B.transpose();
	cout << "矩阵A的逆阵:" << endl;
	cout << A.inversion();
	cout << "90.0125转弧度：" << endl;
	cout << Tool::angleToRadian(90.0125) << endl;
	cout << "转回角度" << endl;
	cout << Tool::radianToAngle(Tool::angleToRadian(90.0125)) << endl;
	cout << "第二象限角平分线方位角:" << endl;
	cout << Tool::radianToAngle(Tool::coordinateToAzimuthAngle(-25.5, 25.5, -51.0, 51)) << endl;
	*/
	LevelApproximation levelA;
	levelA.Calculation();
	LevelPrecision levelP;
	levelP.calculation();
	vector<string> used;
	cout<< "A,B两点最短距离：" << levelP.getMinHeight("A", "B", used) << endl;
	sideAngleAdjust sAA;
	sAA.adjustment();
	angleAdjust AA;
    AA.adjust();
	return 0;
}