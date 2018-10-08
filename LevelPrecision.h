#pragma once
#include "include.h"
#include "Point.h"
#include "SurveyValue.h"
#include "Tool.h"
#include "CMatrix.h"



class LevelPrecision {
public:
	LevelPrecision(const char * path);
	void calculation();
	int searchPointByName(string name);
	double getHeightBySearch(string name,string sign);
	//直接搜索
	double getMinHeight(string name,string end,vector<string> used);
	vector<int> nextLens(string name, vector<string> used);
	bool hasUsed(vector<string> used,string currentName);
	//构造图搜索
private:
	vector<Point> points;
	vector<SurveyValue> surveyValues;
	//中误差
	double mediumError;
	//所有点数
	int allPoints;
	//已知点数
	int knownPoints;
	//所有高差数
	int numsOfEachLen;
	//最近路径
	vector<string> path;
};

LevelPrecision::LevelPrecision(const char * path) {
	/*读取数据*/
	string s;
	fstream f;
	f.open(path);
	//获取第一行
	getline(f, s);
	vector<string> ss = Tool::split(s, ' ');
	allPoints = Tool::toInt(ss.at(0));
	knownPoints = Tool::toInt(ss.at(1));
	numsOfEachLen = Tool::toInt(ss.at(2));
	mediumError = Tool::toDouble(ss.at(3));
	//获取已知点及加入未知点
	for (int i = 0; i < allPoints; i++) {
		if (i < knownPoints) {
			ss.clear();
			getline(f, s);
			ss = Tool::split(s, ' ');
			Point tempPoint(ss.at(0), Tool::toDouble(ss.at(1)));
			points.push_back(tempPoint);
		}
		else {
			Point tempPoint("", -9999.9);
			points.push_back(tempPoint);
		}
	}
	//获取各段数据
	for (int i = 0; i < numsOfEachLen; i++) {
		getline(f, s);
		ss.clear();
		ss = Tool::split(s, ' ');
		searchPointByName(ss.at(0));
		searchPointByName(ss.at(1));
		SurveyValue suv(ss.at(0), ss.at(1), Tool::toDouble(ss.at(2)), Tool::toDouble(ss.at(3)), 0, 0);
		surveyValues.push_back(suv);
	}
	f.close();
}

void LevelPrecision::calculation() {
	//先得到各未知点近似高程，任意已知点起算均可
	for (int i = knownPoints; i < allPoints; i++) {
		points.at(i).height = getHeightBySearch(points.at(i).name,"eachHeight");
	}
	//n个误差方程
	//构造n * n权阵：C / S
	CMatrix<double> P(numsOfEachLen, numsOfEachLen);
	for (int i = 0; i < numsOfEachLen; i++) {
		P(i, i) = 1 / surveyValues.at(i).eachLength;
	}

	//构造V = Bx - l的系数阵B
	CMatrix<double> B(numsOfEachLen, allPoints - knownPoints);
	//常数项矩阵（列向量）
	CMatrix<double> l(numsOfEachLen, 1);
	for (int i = 0; i < numsOfEachLen; i++) {
		int begin = searchPointByName(surveyValues.at(i).begin);
		int end = searchPointByName(surveyValues.at(i).end);
		//begin and end < knownPoints -> 已知
		//刚好原来构造的是全零矩阵，只需添加 1 or -1 ,0无需改
		//B每行的顺序即是points 3，4，5位存储点的顺序
		if (begin >= knownPoints) {
			//已知++
			B(i, begin - knownPoints) = -1;
		}
		if (end >= knownPoints) {
			//已知++
			B(i, end - knownPoints) = 1;
		}
		l(i, 0) = surveyValues.at(i).eachHeight
			-
			(points.at(searchPointByName(surveyValues.at(i).end)).height
			-
			points.at(searchPointByName(surveyValues.at(i).begin)).height);
	}
	//计算得到近似值误差矩阵x
	CMatrix<double> x = (B.transpose() * P * B).inversion() * B.transpose() * P * l;
	CMatrix<double> V = B * x - l;
	cout << V;
	//高差改正
	for (int i = 0; i < numsOfEachLen; i++) {
		surveyValues.at(i).deltaHeight = V(i, 0);
		surveyValues.at(i).eachHeightAfterCorrect = surveyValues.at(i).eachHeight + V(i,0);
	}
	//求出未知点的真高程
	for (int i = knownPoints; i < allPoints; i++) {
		points.at(i).height = getHeightBySearch(points.at(i).name,"eachHeightAfterCorrect");
	}
}

/*通过名称查找点，若无将此点名添加到最后*/
int LevelPrecision::searchPointByName(string name) {
	int result = -1;
	for (unsigned int i = 0; i < points.size(); i++) {
		if (points.at(i).name != "") {
			if (points.at(i).name == name) {
				result = i;
				break;
			}
		}
		else {
			points.at(i).name = name;
			result = i;
			break;
		}
	}
	return result;
}

/*求近似高程或真实高程*/
double LevelPrecision::getHeightBySearch(string name,string sign) {
	if (!(sign == "eachHeight" || sign == "eachHeightAfterCorrect")) {
		cout << "无法求高程！" << endl;
		return -9999.9;
	}
	double result = 0;
	string currentNmae = name;
	string oldName = name;
	//当前点不为已知点则继续查找
	while (searchPointByName(currentNmae) >= knownPoints)
	{
		oldName = currentNmae;
		//先假设currentName为end,查找得到surveyvalues中name为end的begin
		for (int i = 0; i < numsOfEachLen; i++) {
			if (currentNmae == surveyValues.at(i).end) {
				oldName = currentNmae;
				currentNmae = surveyValues.at(i).begin;
				if (sign == "eachHeight")
					result += surveyValues.at(i).eachHeight;
				else
					result += surveyValues.at(i).eachHeightAfterCorrect;
				break;
			}
		}
		//若currentName不在end，则一定在begin;
		if (currentNmae == oldName) {
			for (int i = 0; i < numsOfEachLen; i++) {
				if (currentNmae == surveyValues.at(i).begin) {
					oldName = currentNmae;
					currentNmae = surveyValues.at(i).end;
					if (sign == "eachHeight")
						result += surveyValues.at(i).eachHeight;
					else
						result += surveyValues.at(i).eachHeightAfterCorrect;
					break;
				}
			}
		}
	}
	//加上得到的已知点的高程
	result += points.at(searchPointByName(currentNmae)).height;
	return result;
}

/*下面三个函数递归求解水准网已知点最短路径*/
double LevelPrecision::getMinHeight(string name,string end,vector<string> used) {
	vector<int> nexts = nextLens(name,used);
	if (name == end)
		return 0.0;
	if (nexts.size() == 0)
		return DBL_MAX;
	used.push_back(name);
	string pathEach = nexts.at(0) > 0 ? surveyValues.at(abs(nexts.at(0)) - 1).end : surveyValues.at(abs(nexts.at(0)) - 1).begin;
	double min = surveyValues.at(abs(nexts.at(0)) - 1).eachLength
			+ getMinHeight(nexts.at(0) > 0 ? surveyValues.at(abs(nexts.at(0)) - 1).end : surveyValues.at(abs(nexts.at(0)) - 1).begin, end, used);
	for (int i = 1; i < nexts.size(); i++) {
		double temp = surveyValues.at(abs(nexts.at(i)) - 1).eachLength
			+ getMinHeight(nexts.at(i) > 0 ? surveyValues.at(abs(nexts.at(i)) - 1).end : surveyValues.at(abs(nexts.at(i)) - 1).begin, end, used);
		if (temp < min) {
			pathEach = nexts.at(i) > 0 ? surveyValues.at(abs(nexts.at(i)) - 1).end : surveyValues.at(abs(nexts.at(i)) - 1).begin;
			min = temp;
		}
	}
	if (pathEach != "P3")
		path.push_back(pathEach);
	return min;
}

vector<int> LevelPrecision::nextLens(string name, vector<string> used) {
	vector<int> result;
	//name at begin
	for (int i = 0; i < surveyValues.size(); i++) {
		if (!hasUsed(used, surveyValues.at(i).end) && surveyValues.at(i).begin == name)
			result.push_back(i + 1);
	}
	//name at end
	for (int i = 0; i < surveyValues.size(); i++) {
		if (!hasUsed(used, surveyValues.at(i).begin) && surveyValues.at(i).end == name)
			result.push_back(-(i + 1));
	}
	return result;
}

bool LevelPrecision::hasUsed(vector<string> used,string currentName) {
	for (int i = 0; i < used.size(); i++) {
		if (used.at(i) == currentName)
			return true;
	}
	return false;
}
