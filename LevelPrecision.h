#pragma once
#include "include.h"
#include "Point.h"
#include "SurveyValue.h"
#include "Tool.h"
#include "CMatrix.h"



class LevelPrecision {
public:
	LevelPrecision();
	void calculation();
	int searchPointByName(string name);
	double getHeightBySearch(string name,string sign);
	//ֱ������
	double getMinHeight(string name,string end,vector<string> used);
	vector<int> nextLens(string name, vector<string> used);
	bool hasUsed(vector<string> used,string currentName);
private:
	vector<Point> points;
	vector<SurveyValue> surveyValues;
	//�����
	double mediumError;
	//���е���
	int allPoints;
	//��֪����
	int knownPoints;
	//���и߲���
	int numsOfEachLen;
	//���·��
	vector<string> path;
};

LevelPrecision::LevelPrecision() {
	/*��ȡ����*/
	string s;
	fstream f;
	cout << "������ˮ׼����ʼ����·����eg:D:/levelNet.txt),Ĭ��Ϊ��ǰ·���µ�levelNet.txt:" << endl;
	string path = "";
	getline(cin, path);
	if (path == "") {
		path = "levelNet.txt";
		bool exist = Tool::fileExist(path);
		while (!exist) {
			cout << "·����Ч�����������룺";
			getline(cin, path);
			exist = Tool::fileExist(path);
		}
		f.open(path);
	}
	else
	{
		bool exist = Tool::fileExist(path);
		while (!exist) {
			cout << "·����Ч�����������룺";
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
	//��ȡ��һ��
	getline(f, s);
	vector<string> ss = Tool::split(s, ' ');
	allPoints = Tool::toInt(ss.at(0));
	knownPoints = Tool::toInt(ss.at(1));
	numsOfEachLen = Tool::toInt(ss.at(2));
	mediumError = Tool::toDouble(ss.at(3));
	//��ȡ��֪�㼰����δ֪��
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
	//��ȡ��������
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
	//�ȵõ���δ֪����Ƹ̣߳�������֪���������
	for (int i = knownPoints; i < allPoints; i++) {
		points.at(i).height = getHeightBySearch(points.at(i).name,"eachHeight");
	}
	//n������
	//����n * nȨ��C / S
	CMatrix<double> P(numsOfEachLen, numsOfEachLen);
	for (int i = 0; i < numsOfEachLen; i++) {
		P(i, i) = 1 / surveyValues.at(i).eachLength;
	}

	//����V = Bx - l��ϵ����B
	CMatrix<double> B(numsOfEachLen, allPoints - knownPoints);
	//�����������������
	CMatrix<double> l(numsOfEachLen, 1);
	for (int i = 0; i < numsOfEachLen; i++) {
		int begin = searchPointByName(surveyValues.at(i).begin);
		int end = searchPointByName(surveyValues.at(i).end);
		//begin and end < knownPoints -> ��֪
		//�պ�ԭ���������ȫ�����ֻ����� 1 or -1 ,0�����
		//Bÿ�е�˳����points 3��4��5λ�洢���˳��
		if (begin >= knownPoints) {
			//��֪++
			B(i, begin - knownPoints) = -1;
		}
		if (end >= knownPoints) {
			//��֪++
			B(i, end - knownPoints) = 1;
		}
		l(i, 0) = surveyValues.at(i).eachHeight
			-
			(points.at(searchPointByName(surveyValues.at(i).end)).height
			-
			points.at(searchPointByName(surveyValues.at(i).begin)).height);
	}
	//����õ�����ֵ������x
	CMatrix<double> x = (B.transpose() * P * B).inversion() * B.transpose() * P * l;
	CMatrix<double> V = B * x - l;
	//�߲����
	for (int i = 0; i < numsOfEachLen; i++) {
		surveyValues.at(i).deltaHeight = V(i, 0);
		surveyValues.at(i).eachHeightAfterCorrect = surveyValues.at(i).eachHeight + V(i,0);
	}
	//���δ֪�����߳�
	for (int i = knownPoints; i < allPoints; i++) {
		points.at(i).height = getHeightBySearch(points.at(i).name,"eachHeightAfterCorrect");
	}

	//���ƽ����
	ofstream of;
	cout << "������ˮ׼��ƽ�������·����eg:D:/levelNetResult.txt;Ĭ���ڵ�ǰ·��levelNetResult.txt����" << endl;
	string resultPath = "";
	getline(cin, resultPath);
	if (resultPath == "")
		of.open("levelNetResult.txt");
	else
		of.open(resultPath);
	of.flags(ios::left);
	of << "ˮ׼��ƽ����" << endl;
	of << setw(20) << "����" << setw(20) << "·�߳�" << setw(20) << "�۲�߲�" << setw(20)
		<< "������" << setw(20) << "������߲� " << setw(20) << "�߳�" << endl;

	//д�����н��
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

/*ͨ�����Ʋ��ҵ㣬���޽��˵�����ӵ����*/
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

/*����Ƹ̻߳���ʵ�߳�*/
double LevelPrecision::getHeightBySearch(string name,string sign) {
	if (!(sign == "eachHeight" || sign == "eachHeightAfterCorrect")) {
		cout << "�޷���̣߳�" << endl;
		return -9999.9;
	}
	double result = 0;
	string currentNmae = name;
	string oldName = name;
	//��ǰ�㲻Ϊ��֪�����������
	while (searchPointByName(currentNmae) >= knownPoints)
	{
		oldName = currentNmae;
		//�ȼ���currentNameΪend,���ҵõ�surveyvalues��nameΪend��begin
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
		//��currentName����end����һ����begin;
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
	//���ϵõ�����֪��ĸ߳�
	result += points.at(searchPointByName(currentNmae)).height;
	return result;
}

/*�������������ݹ����ˮ׼����֪�����·��*/
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
