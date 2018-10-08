#pragma once
#include "include.h"

//析构函数不能是模板
//模板必须写在一个文件的类的大括号内
//=重载为“成员”函数且参数“const” and “&”
//内部每个函数无需添加 tmeplate <class T>
//函数返回的临时对象可以与引用绑定直到析构
template <class T>
class CMatrix
{
public:
	/*无参构造空矩阵*/
	CMatrix() {
		m_rows = 0;
		m_columns = 0;
		m_matrix = NULL;
	};

	/*从文件获取矩阵*/
	CMatrix(const char * path) {
		vector<T> temp;
		ifstream f;
		f.open(path, ios::in);
		char c;
		T d;
		m_rows = 0;
		m_columns = 0;
		while (f.peek() != EOF) {
			if (f.peek() == '\n' || f.peek() == '\r\n') {
				m_rows++;
				m_columns = 0;
			}
			f >> d >> c;
			temp.push_back(d);
			m_columns++;
		}
		//当最后一行不为空
		m_rows++;
		m_matrix = new T[temp.size()];
		memcpy(m_matrix, &temp[0], temp.size() * sizeof(T));
	}

	/*构造全零矩阵*/
	CMatrix(int rows, int columns) :m_rows(rows), m_columns(columns) {
		m_matrix = new T[rows*columns];
		for (int i = 0; i < rows*columns; i++) {
			m_matrix[i] = 0;
		}
	}

	/*拷贝构造*/
	CMatrix(const CMatrix& matrix) {
		m_rows = matrix.m_rows;
		m_columns = matrix.m_columns;
		m_matrix = new T[m_rows * m_columns];
		for (int i = 0; i < m_rows * m_columns; i++) {
			m_matrix[i] = matrix.m_matrix[i];
		}
	}

	/*获取任一矩阵元素*/
	T& operator() (int row, int column) {
		return m_matrix[row * m_columns + column];
	}

	/*获取任一矩阵元素*/
	T& operator() (int row, int column) const{
		return m_matrix[row * m_columns + column];
	}

	/*加号重载*/
	friend CMatrix<T> operator + (const CMatrix<T>& matrixA,const CMatrix<T>& matrixB) {
		CMatrix<T> result(matrixA.m_rows, matrixA.m_columns);
		if (matrixA.m_rows != matrixB.m_rows || matrixA.m_columns != matrixB.m_columns) {
			cout << "无法相加!" << endl;
			return result;
		}
		for (int i = 0; i < matrixA.m_rows; i++) {
			for (int j = 0; j < matrixA.m_columns; j++) {
				result(i, j) = matrixA(i, j) + matrixB(i, j);
			}
		}
		return result;
	}

	/*减号重载*/
	friend CMatrix<T> operator - (const CMatrix<T>& matrixA,const CMatrix<T>& matrixB) {
		CMatrix<T> result(matrixA.m_rows, matrixA.m_columns);
		if (matrixA.m_rows != matrixB.m_rows || matrixA.m_columns != matrixB.m_columns) {
			cout << "无法相减!" << endl;
			return result;
		}
		for (int i = 0; i < matrixA.m_rows; i++) {
			for (int j = 0; j < matrixA.m_columns; j++) {
				result(i, j) = matrixA(i, j) - matrixB(i, j);
			}
		}
		return result;
	}

	/*乘号重载*/
	friend CMatrix<T> operator * (const CMatrix<T>& matrixA, const CMatrix<T>& matrixB) {
		CMatrix<T> matrixResult(matrixA.m_rows, matrixB.m_columns);
		if (matrixA.m_columns != matrixB.m_rows) {
			cout << "无法相乘!" << endl;
			return matrixResult;
		}
		for (int i = 0; i < matrixA.m_rows; i++) {
			//B列
			for (int j = 0; j < matrixB.m_columns; j++) {
				T result = 0;
				//B列的每一个元素
				for (int k = 0; k < matrixB.m_rows; k++) {
					result += matrixA(i, k) * matrixB(k, j);
				}
				matrixResult(i, j) = result;
			}
		}
		return matrixResult;
	}

	/*转置*/
	CMatrix<T> transpose() {
		CMatrix result(m_columns, m_rows);
		for (int i = 0; i < m_rows; i++) {
			for (int j = 0; j < m_columns; j++) {
				result(j, i) = m_matrix[i*m_columns + j];
			}
		}
		return result;
	}

	/*求逆*/
	CMatrix<T> inversion() {
		if (m_rows != m_columns) {
			cout<<"无法求逆"<<endl;
			return NULL;
		}

		//取得上三角阵
		CMatrix T1(m_rows, m_columns);
		T element00 = sqrt((double)m_matrix[0]);
		T1(0, 0) = element00;
		for (int i = 1; i < m_columns; i++) {
			T1(0, i) = m_matrix[i] / element00;
		}
		//行
		for (int i = 1; i < m_columns; i++) {
			//对角线元素
			T temp1 = 0;
			for (int j = 0; j < i; j++) {
				temp1 += T1(j, i) * T1(j, i);
			}
			T temp2 = sqrt((double)(m_matrix[i*m_columns + i] - temp1));
			T1(i, i) = temp2;

			//每行对角线右侧的元素遍历
			for (int j = i + 1; j < m_columns; j++) {
				T temp3 = 0;
				for (int k = 0; k < i; k++) {
					temp3 += T1(k, i) * T1(k, j);
				}
				T1(i, j) = (m_matrix[i*m_columns + j] - temp3) / temp2;
			}
		}

		CMatrix R(m_rows, m_columns);
		for (int i = m_rows - 1; i >= 0; i--) {
			R(i, i) = 1 / T1(i, i);
			for (int j = i + 1; j < m_columns; j++) {
				T sum = 0;
				for (int k = i + 1; k <= j; k++) {
					sum += T1(i, k) * R(k, j);
				}
				R(i, j) = -sum / T1(i, i);
			}
		}

		CMatrix temp;
		temp = R.transpose();
		CMatrix result;
		result= R * temp;
		return result;
	}

	/*输出重载*/
	friend ostream& operator << (ostream& out,const CMatrix<T> &matrix) {

		for (int i = 0; i < matrix.m_rows; i++) {
			for (int j = 0; j < matrix.m_columns; j++) {
				out << matrix(i, j) << ", ";
			}
			out << endl;
		}
		out << "**************" << endl;
		return out;
	}

	/*赋值重载*/
	CMatrix<T>& operator= (const CMatrix &matrix) {
		m_rows = matrix.m_rows;
		m_columns = matrix.m_columns;
		delete[]m_matrix;
		m_matrix = new T[m_rows * m_columns];
		for (int i = 0; i < m_rows * m_columns; i++) {
			m_matrix[i] = matrix.m_matrix[i];
		}
		return *this;
	}

	/*析构*/
	~CMatrix() {
		delete[]m_matrix;
	}

private:
	int m_rows;
	int m_columns;
	T * m_matrix;
};



