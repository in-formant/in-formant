#ifndef MATRIX_H
#define MATRIX_H

#include <string>

using namespace std;

class Matrix{
private:
	int number_rows;
	int number_columns;

	int **index_M;
	int **index_N;
public:
	Matrix();
	~Matrix();

	void Inverse(string type_matrix, int number_rows, float **M, float **N);
	void Inverse(string type_matrix, int number_rows, double **M, double **N);
	void Multiplication(int M_rows, int M_columns, int N_columns, float **M, float **N, float **O);
	void Multiplication(int M_rows, int M_columns, int N_columns, double **M, double **N, double **O);
	void Transpose(int number_rows, int number_columns, float **M, float **N);
	void Transpose(int number_rows, int number_columns, double **M, double **N);

	int LU_Decomposition(int number_rows, float **M, float **L, float **U);
	int LU_Decomposition(int number_rows, double **M, double **L, double **U);

	float Determinant(string type_matrix, int number_rows, float **M);

	double Determinant(string type_matrix, int number_rows, double **M);
};

#endif
