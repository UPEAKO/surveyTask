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
	LevelApproximation(const char * path) {
		/*读取数据*/
		string s;
		fstream f;
		f.open(path);
		//获取第一行
		getline(f, s);
		vector<string> ss = Tool::split(s, ',');
		int allPoints = Tool::toInt(ss.at(0));
		int knownPoints = Tool::toInt(ss.at(1));
		int numsOfEachLen = Tool::toInt(ss.at(2));
		//获取已知点及加入未知点
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
		//将近似平差结果写入文件
		ofstream of;
		of.open("E:/cpp/data/LevelApproximation.txt");
		of.flags(ios::left);
		of << setw(20) << "点名" << setw(20) << "路线长" << setw(20) << "观测高差" << setw(20) 
			<< "改正数" << setw(20) << "改正后高差 " << setw(20) << "高程" << endl;
		//B放到最后
		Point temp = points.at(1);
		points.erase(points.begin() + 1);
		points.push_back(temp);
		//写出所有结果
		//设置小数点后三位，会四舍五入
		of << setiosflags(ios::fixed) << setprecision(3);
		for (unsigned int i = 0; i < surveyValues.size(); i++) {
			of << setw(20) << points.at(i).name 
				<< setw(20) << surveyValues.at(i).eachLength 
				<< setw(20) << surveyValues.at(i).eachHeight
				<< setw(20) << surveyValues.at(i).deltaHeight 
				<< setw(20) << surveyValues.at(i).eachHeightAfterCorrect 
				<<setw(20) << points.at(i).height << endl;
		}
		of << setw(20) << points.at(points.size() - 1).name 
			<< setw(20) << sumLength 
			<< setw(20) << sumHeight
			<< setw(20) << deltaHeightSum
			<< setw(20) << deltaHeight 
			<< setw(20) << points.at(points.size() - 1).height << endl;
		of.close();
		cout << "水准近似平差结果写入文件\"E:/cpp/data/LevelApproximation.txt\"中!" << endl;
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