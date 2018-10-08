#pragma once
#include "include.h"

class Tool {
public:
	static double angleToRadian(double angle) {
		int degree = (int)angle;
		double temp = (angle - degree) * 100;
		int minute = (int)temp;
		double second = (temp - minute) * 100;
		return (degree * 60 * 60 + minute * 60 + second) / (180 * 60 * 60) * PI;
	}

	static double coordinateToAzimuthAngle(double x1, double y1, double x2, double y2) {
		double result = 0;
		double deltaX = x2 - x1;
		double deltaY = y2 - y1;
		//deltaX == 0 时
		if (fabs(deltaX) < 1.0E-9) {
			if (deltaY > 1.0E-9)
				result = PI / 2;
			else if (deltaY < -1.0E-9)
				result = PI * 3 / 2;
		}
		else {
			//数学上弧度值
			double angleInMath = atan(deltaY / deltaX);
			if (deltaX > 1.0E-9) {
				if (deltaY > 1.0E-9)
					result = angleInMath;
				else if (deltaY < -1.0E-9)
					result = PI * 2 + angleInMath;
			}
			else if (deltaX < -1.0E-9)
				result = PI + angleInMath;
		}
		return result;
	}

	static double radianToAngle(double radian) {
		double result = 0;
		double allSecond = (radian / PI) * (180 * 60 * 60);
		//度
		int degree = (int)allSecond / (60 * 60);
		double temp = allSecond - degree * 60 * 60;
		//分
		int minute = (int)temp / 60;
		//秒
		double secondDetial = temp - minute * 60;
		int second = (int)secondDetial;
		/*厘秒*/
		double centisecondDetail = (secondDetial - second) * 100;
		int centisecond = (int)(centisecondDetail + 0.5);
		//厘秒不进位
		if (centisecond < 100) {
			result = degree + minute / 1E2 + second / 1E4 + centisecond / 1E6;
		}
		//厘秒进位
		else {
			//秒进位
			if (second == 59) {
				//分进位
				if (minute == 59) {
					result = degree + 1;
				}
				//分不进位
				else {
					result = degree + (minute + 1) / 1E2;
				}
			}
			//秒不进位
			else {
				result = degree + minute / 1E2 + (second + 1) / 1E4;
			}
		}
		return result;
	}

	static vector<string> split(string ss, char sign) {
		vector<string> result;
		unsigned int start = 0, next = 0;
		start = ss.find(sign);
		if (start == 0) {
			start++;
		}
		else
		{
			start = 0;
		}
		while (start < ss.length())
		{
			next = ss.find(sign, start);
			if (next != -1) {
				string s = ss.substr(start, next - start);
				result.push_back(s);
				start = next + 1;
			}
			else
				break;
		}
		if (start < ss.length()) {
			string s = ss.substr(start, ss.length() - start);
			result.push_back(s);
		}
		return result;
	}

	static int toInt(string ss) {
		istringstream is(ss);
		int result;
		is >> result;
		return result;
	}

	static double toDouble(string ss) {
		istringstream is(ss);
		double result;
		is >> result;
		return result;
	}

	static double angle2MinusAngle1(double angle2, double angle1) {
		return angle2 * 180 * 60 * 60 - angle1 * 180 * 60 * 60;
	}
};