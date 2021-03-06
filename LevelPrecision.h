#pragma once
#include "include.h"
#include "Point.h"
#include "SurveyValue.h"
#include "Tool.h"
#include "CMatrix.h"
#include "MinLenInfo.h"


class LevelPrecision {
public:
	LevelPrecision();
	void calculation();
	int searchPointByName(string name);
	double getHeightBySearch(string name,string sign);
	//直接搜索
	MinLenInfo getMinHeight(string name,string end,vector<string> used);
	vector<int> nextLens(string name, vector<string> used);
	bool hasUsed(vector<string> used,string currentName);
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
	vector<string> finalPath;
};

LevelPrecision::LevelPrecision() {
	/*读取数据*/
	string s;
	fstream f;
	cout << "请输入水准网初始数据路径（eg:D:/levelNet.txt),默认为当前路径下的levelNet.txt:" << endl;
	string path = "";
	getline(cin, path);
	if (path == "") {
		path = "levelNet.txt";
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
				path = "levelNet.txt";
				exist = Tool::fileExist(path);
			}
			else {
				exist = Tool::fileExist(path);
			}
		}
		f.open(path);
	}
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
	//高差改正
	for (int i = 0; i < numsOfEachLen; i++) {
		surveyValues.at(i).deltaHeight = V(i, 0);
		surveyValues.at(i).eachHeightAfterCorrect = surveyValues.at(i).eachHeight + V(i,0);
	}
	//求出未知点的真高程
	for (int i = knownPoints; i < allPoints; i++) {
		points.at(i).height = getHeightBySearch(points.at(i).name,"eachHeightAfterCorrect");
	}

	//输出平差结果
	ofstream of;
	cout << "请设置水准网平差结果输出路径（eg:D:/levelNetResult.txt;默认在当前路径levelNetResult.txt）：" << endl;
	string resultPath = "";
	getline(cin, resultPath);
	if (resultPath == "")
		of.open("levelNetResult.txt");
	else
		of.open(resultPath);
	of.flags(ios::left);
	of << "水准网平差结果" << endl;
	of << endl;

	//闭合差
	vector<string> used;
	MinLenInfo minInfo = getMinHeight(points.at(0).name, points.at(1).name, used);
	finalPath = minInfo.path;
	finalPath.push_back(points.at(0).name);
	double sumHeight = 0;
	for (int i = 0; i < finalPath.size() - 1; i++) {
		for (int j = 0; j < surveyValues.size(); j++) {
			if (surveyValues.at(j).begin == finalPath.at(i) && surveyValues.at(j).end == finalPath.at(i + 1))
				sumHeight -= surveyValues.at(j).eachHeight;
			else if (surveyValues.at(j).end == finalPath.at(i) && surveyValues.at(j).begin == finalPath.at(i + 1))
				sumHeight += surveyValues.at(j).eachHeight;
		}
	}
	double deltaHeight = (points.at(1).height - points.at(0).height) - sumHeight;
	of << "闭合差: " << deltaHeight << endl;
	of << endl;

	//后验中误差
	CMatrix<double> lateMatrix = V.transpose() * P * V;
	double lateError = sqrt(lateMatrix(0, 0) / (numsOfEachLen - (allPoints - knownPoints)));
	of << "后验单位权中误差：" << lateError << endl;
	of << endl;

	of << setw(20) << "点名" << setw(20) << "路线长" << setw(20) << "观测高差" << setw(20)
		<< "改正数" << setw(20) << "改正后高差 " << setw(20) << "高程" << endl;

	//写出所有结果
	of << setiosflags(ios::fixed) << setprecision(3);
	for (unsigned int i = 0; i < surveyValues.size(); i++) {
		of << setw(100) << surveyValues.at(i).begin << setw(20) << points.at(searchPointByName(surveyValues.at(i).begin)).height << endl;
		of << setw(20) << surveyValues.at(i).end
			<< setw(20) << surveyValues.at(i).eachLength
			<< setw(20) << surveyValues.at(i).eachHeight
			<< setw(20) << surveyValues.at(i).deltaHeight
			<< setw(20) << surveyValues.at(i).eachHeightAfterCorrect 
			<< setw(20) << points.at(searchPointByName(surveyValues.at(i).end)).height << endl;
		of << endl;
	}
	of.close();
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
MinLenInfo LevelPrecision::getMinHeight(string name,string end,vector<string> used) {
	vector<int> nexts = nextLens(name,used);
	if (name == end) {
		MinLenInfo minInfo;
		minInfo.min = 0.0;
		return minInfo;
	}
	if (nexts.size() == 0) {
		MinLenInfo minInfo;
		minInfo.min = DBL_MAX;
		return minInfo;
	}
	used.push_back(name);

	string pathEach = nexts.at(0) > 0 ? surveyValues.at(abs(nexts.at(0)) - 1).end : surveyValues.at(abs(nexts.at(0)) - 1).begin;
	MinLenInfo minInfo = getMinHeight(nexts.at(0) > 0 ? surveyValues.at(abs(nexts.at(0)) - 1).end : surveyValues.at(abs(nexts.at(0)) - 1).begin, end, used);
	minInfo.min += surveyValues.at(abs(nexts.at(0)) - 1).eachLength;
	minInfo.path.push_back(pathEach);

	for (int i = 1; i < nexts.size(); i++) {
		string tempPathEach = nexts.at(i) > 0 ? surveyValues.at(abs(nexts.at(i)) - 1).end : surveyValues.at(abs(nexts.at(i)) - 1).begin;
		MinLenInfo tempMinInfo = getMinHeight(nexts.at(i) > 0 ? surveyValues.at(abs(nexts.at(i)) - 1).end : surveyValues.at(abs(nexts.at(i)) - 1).begin, end, used);
		tempMinInfo.min += surveyValues.at(abs(nexts.at(i)) - 1).eachLength;
		tempMinInfo.path.push_back(tempPathEach);
		if (tempMinInfo.min < minInfo.min) {
			minInfo = tempMinInfo;
		}
	}
	return minInfo;
}

vector<int> LevelPrecision::nextLens(string name, vector<string> used) {
	//由于正负零问题，故加一
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
