#pragma once
#include "include.h"
#include "Point.h"
#include "SurveyValue.h"
#include "Tool.h"

class LevelApproximation {
private:
	vector<Point> points;
	vector<SurveyValue> surveyValues;
public:
	LevelApproximation() {
		/*读取数据*/
		string s;
		fstream f;
		cout << "请输入水准路线初始数据路径（eg:D:/level.txt),默认为当前路径下的level.txt:" << endl;
		string path = "";
		getline(cin, path);
		if (path == "") {
			path = "level.txt";
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
					path = "level.txt";
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
		vector<string> ss = Tool::split(s, ',');
		int allPoints = Tool::toInt(ss.at(0));
		int knownPoints = Tool::toInt(ss.at(1));
		int numsOfEachLen = Tool::toInt(ss.at(2));
		//获取已知点及未知点置为非法
		for (int i = 0; i < allPoints; i++) {
			if (i < knownPoints) {
				ss.clear();
				getline(f, s);
				ss = Tool::split(s, ',');
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
			ss = Tool::split(s, ',');
			SurveyValue suv(ss.at(0), ss.at(1), Tool::toDouble(ss.at(2)), Tool::toDouble(ss.at(3)), 0, 0);
			surveyValues.push_back(suv);
		}
		f.close();
	}

	void Calculation() {
		double sumLength = 0;
		double sumHeight = 0;
		for (SurveyValue temp :
		surveyValues) {
			sumLength += temp.eachLength;
			sumHeight += temp.eachHeight;
		}
		//闭合差
		double deltaHeight = (points.at(1).height - points.at(0).height);
		double deltaHeightSum = deltaHeight - sumHeight;
		if (deltaHeightSum > 1E-3) {
			cout << "闭合差超限!" << endl;
			return;
		}
		//每段高差改正数及改正后高差
		for (unsigned int i = 0; i < surveyValues.size(); i++) {
			double eachDeltaHeight = deltaHeightSum * surveyValues.at(i).eachLength / sumLength;
			surveyValues.at(i).deltaHeight = eachDeltaHeight;
			surveyValues.at(i).eachHeightAfterCorrect = surveyValues.at(i).eachHeight + eachDeltaHeight;
		}
		//依据surveyValues计算每个点的实际高程
		for (unsigned int i = 0; i < surveyValues.size() - 1; i++) {
			int begin = searchPointByName(surveyValues.at(i).begin);
			int end = searchPointByName(surveyValues.at(i).end);
			points.at(end).height = points.at(begin).height + surveyValues.at(i).eachHeightAfterCorrect;
		}
		//输出平差结果
		ofstream of;
		cout << "请设置水准路线平差结果输出路径（eg:D:/levelResult.txt;默认在当前路径levelResult.txt）：" << endl;
		string resultPath = "";
		getline(cin, resultPath);
		if (resultPath == "")
			of.open("levelResult.txt");
		else
			of.open(resultPath);
		of.flags(ios::left);
		of << "水准路线平差结果" << endl;
		of << setw(20) << "点名" << setw(20) << "路线长" << setw(20) << "观测高差" << setw(20) 
			<< "改正数" << setw(20) << "改正后高差 " << setw(20) << "高程" << endl;
		//写出所有结果
		of << setiosflags(ios::fixed) << setprecision(3);
		for (unsigned int i = 0; i < surveyValues.size(); i++) {
			of << setw(100) <<surveyValues.at(i).begin << setw(20) << points.at(searchPointByName(surveyValues.at(i).begin)).height << endl;
			of << setw(20) << surveyValues.at(i).end
				<< setw(20) << surveyValues.at(i).eachLength 
				<< setw(20) << surveyValues.at(i).eachHeight
				<< setw(20) << surveyValues.at(i).deltaHeight 
				<< setw(20) << surveyValues.at(i).eachHeightAfterCorrect 
				<< setw(20) << points.at(searchPointByName(surveyValues.at(i).end)).height <<endl;
			of << endl;
		}
		of.close();
	}

	int searchPointByName(string name) {
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
};