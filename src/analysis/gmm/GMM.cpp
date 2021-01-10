#include <fstream>
#include <iostream>
#include <cmath>

#include "GMM.h"
#include "Kmeans.h"
#include "Matrix.h"

Gaussian_Mixture_Model::Gaussian_Mixture_Model(string type_covariance, int dimension_data, int number_gaussian_components){
	this->type_covariance = type_covariance;
	this->dimension_data = dimension_data;
	this->number_gaussian_components = number_gaussian_components;

	mean = new double*[number_gaussian_components];
	weight = new double[number_gaussian_components];

	for (int i = 0; i < number_gaussian_components; i++){
		mean[i] = new double[dimension_data];
	}

	if (!type_covariance.compare("diagonal")){
		diagonal_covariance = new double*[number_gaussian_components];

		for (int i = 0; i < number_gaussian_components; i++){
			diagonal_covariance[i] = new double[dimension_data];
		}
	}
	else{
		covariance = new double**[number_gaussian_components];

		for (int i = 0; i < number_gaussian_components; i++){
			covariance[i] = new double*[dimension_data];

			for (int j = 0; j < dimension_data; j++){
				covariance[i][j] = new double[dimension_data];
			}
		}
	}
}
Gaussian_Mixture_Model::~Gaussian_Mixture_Model(){
	if (!type_covariance.compare("diagonal")){
		for (int i = 0; i < number_gaussian_components; i++){
			delete[] diagonal_covariance[i];
		}
		delete[] diagonal_covariance;
	}
	else{
		for (int i = 0; i < number_gaussian_components; i++){
			for (int j = 0; j < dimension_data; j++){
				delete[] covariance[i][j];
			}
			delete[] covariance[i];
		}
		delete[] covariance;
	}

	for (int i = 0; i < number_gaussian_components; i++){
		delete[] mean[i];
	}
	delete[] mean;
	delete[] weight;
}

void Gaussian_Mixture_Model::Initialize(int number_data, double **data){
	KMeans kmeans = KMeans(dimension_data, number_gaussian_components);

	kmeans.Initialize(number_data, data);
	while (kmeans.Cluster(number_data, data));

	for (int i = 0; i < number_gaussian_components; i++){
		for (int j = 0; j < dimension_data; j++){
			if (!type_covariance.compare("diagonal")){
				diagonal_covariance[i][j] = 1;
			}
			else{
				for (int k = 0; k < dimension_data; k++){
					covariance[i][j][k] = (j == k);
				}
			}
		}
		for (int j = 0; j < dimension_data; j++){
			mean[i][j] = kmeans.centroid[i][j];
		}
		weight[i] = 1.0 / number_gaussian_components;
	}
}
void Gaussian_Mixture_Model::Load_Parameter(string path){
	ifstream file(path);

	if (file.is_open()){
		for (int i = 0; i < number_gaussian_components; i++){
			file >> weight[i];
		}

		for (int i = 0; i < number_gaussian_components; i++){
			for (int j = 0; j < dimension_data; j++){
				file >> mean[i][j];
			}
		}

		if (!type_covariance.compare("diagonal")){
			for (int i = 0; i < number_gaussian_components; i++){
				for (int j = 0; j < dimension_data; j++){
					file >> diagonal_covariance[i][j];
				}
			}
		}
		else{
			for (int i = 0; i < number_gaussian_components; i++){
				for (int j = 0; j < dimension_data; j++){
					for (int k = 0; k < dimension_data; k++){
						file >> covariance[i][j][k];
					}
				}
			}
		}
		file.close();
	}
	else{
		cerr << "[Load_Parameter], " + path + " not found" << endl;
	}
}
void Gaussian_Mixture_Model::Save_Parameter(string path){
	ofstream file(path);

	for (int i = 0; i < number_gaussian_components; i++){
		file << weight[i] << endl;
	}

	for (int i = 0; i < number_gaussian_components; i++){
		for (int j = 0; j < dimension_data; j++){
			file << mean[i][j] << endl;
		}
	}

	if (!type_covariance.compare("diagonal")){
		for (int i = 0; i < number_gaussian_components; i++){
			for (int j = 0; j < dimension_data; j++){
				file << diagonal_covariance[i][j] << endl;
			}
		}
	}
	else{
		for (int i = 0; i < number_gaussian_components; i++){
			for (int j = 0; j < dimension_data; j++){
				for (int k = 0; k < dimension_data; k++){
					file << covariance[i][j][k] << endl;
				}
			}
		}
	}
	file.close();
}

int Gaussian_Mixture_Model::Classify(double data[]){
	int argmax = -1;

	double max = 0;

	for (int i = 0; i < number_gaussian_components; i++){
		double likelihood = weight[i] * Gaussian_Distribution(data, i);

		if (max < likelihood){
			argmax = i;
			max = likelihood;
		}
	}
	return argmax;
}

double Gaussian_Mixture_Model::Calculate_Likelihood(double data[]){
	double likelihood = 0;

	for (int i = 0; i < number_gaussian_components; i++){
		likelihood += weight[i] * Gaussian_Distribution(data, i);
	}
	return likelihood;
}
double Gaussian_Mixture_Model::Calculate_Likelihood(double data[], double gaussian_distribution[]){
	double likelihood = 0;

	for (int i = 0; i < number_gaussian_components; i++){
		likelihood += weight[i] * gaussian_distribution[i];
	}
	return likelihood;
}
double Gaussian_Mixture_Model::Expectaion_Maximization(int number_data, double **data){
	double log_likelihood = 0;

	double *gaussian_distribution = new double[number_gaussian_components];
	double *sum_likelihood = new double[number_gaussian_components];

	double **new_mean = new double*[number_gaussian_components];

	double **new_diagonal_covariance = 0;
	double ***new_covariance = 0;

	for (int i = 0; i < number_gaussian_components; i++){
		new_mean[i] = new double[dimension_data];

		for (int j = 0; j < dimension_data; j++){
			new_mean[i][j] = 0;
		}
		sum_likelihood[i] = 0;
	}

	if (!type_covariance.compare("diagonal")){
		new_diagonal_covariance = new double*[number_gaussian_components];

		for (int i = 0; i < number_gaussian_components; i++){
			new_diagonal_covariance[i] = new double[dimension_data];

			for (int j = 0; j < dimension_data; j++){
				new_diagonal_covariance[i][j] = 0;
			}
		}
	}
	else{
		new_covariance = new double**[number_gaussian_components];

		for (int i = 0; i < number_gaussian_components; i++){
			new_covariance[i] = new double*[dimension_data];

			for (int j = 0; j < dimension_data; j++){
				new_covariance[i][j] = new double[dimension_data];

				for (int k = 0; k < dimension_data; k++){
					new_covariance[i][j][k] = 0;
				}
			}
		}
	}

	for (int i = 0; i < number_data; i++){
		double sum = 0;

		for (int j = 0; j < number_gaussian_components; j++){
			if (!type_covariance.compare("diagonal")){
				sum += weight[j] * (gaussian_distribution[j] = Gaussian_Distribution(data[i], mean[j], diagonal_covariance[j]));
			}
			else{
				sum += weight[j] * (gaussian_distribution[j] = Gaussian_Distribution(data[i], mean[j], covariance[j]));
			}
		}
		for (int j = 0; j < number_gaussian_components; j++){
			double likelihood = weight[j] * gaussian_distribution[j] / sum;

			for (int k = 0; k < dimension_data; k++){
				if (!type_covariance.compare("diagonal")){
					new_diagonal_covariance[j][k] += likelihood * (data[i][k] - mean[j][k]) * (data[i][k] - mean[j][k]);
				}
				else{
					for (int l = 0; l < dimension_data; l++){
						new_covariance[j][k][l] += likelihood * (data[i][k] - mean[j][k]) * (data[i][l] - mean[j][l]);
					}
				}
				new_mean[j][k] += likelihood * data[i][k];
			}
			sum_likelihood[j] += likelihood;
		}
	}

	for (int i = 0; i < number_gaussian_components; i++){
		for (int j = 0; j < dimension_data; j++){
			if (!type_covariance.compare("diagonal")){
				diagonal_covariance[i][j] = new_diagonal_covariance[i][j] / sum_likelihood[i];
			}
			else{
				for (int k = 0; k < dimension_data; k++){
					covariance[i][j][k] = new_covariance[i][j][k] / sum_likelihood[i];
				}
			}
			mean[i][j] = new_mean[i][j] / sum_likelihood[i];
		}
		weight[i] = sum_likelihood[i] / number_data;
	}

	for (int i = 0; i < number_data; i++){
		log_likelihood += log(Calculate_Likelihood(data[i]));
	}

	if (!type_covariance.compare("diagonal")){
		for (int i = 0; i < number_gaussian_components; i++){
			delete[] new_diagonal_covariance[i];
		}
		delete[] new_diagonal_covariance;
	}
	else{
		for (int i = 0; i < number_gaussian_components; i++){
			for (int j = 0; j < dimension_data; j++){
				delete[] new_covariance[i][j];
			}
			delete[] new_covariance[i];
		}
		delete[] new_covariance;
	}

	for (int i = 0; i < number_gaussian_components; i++){
		delete[] new_mean[i];
	}
	delete[] gaussian_distribution;
	delete[] new_mean;
	delete[] sum_likelihood;

	return log_likelihood;
}
double Gaussian_Mixture_Model::Gaussian_Distribution(double data[], int component_index){
	int j = component_index;

	if (!type_covariance.compare("diagonal")){
		return Gaussian_Distribution(data, mean[j], diagonal_covariance[j]);
	}
	else{
		return Gaussian_Distribution(data, mean[j], covariance[j]);
	}
}
double Gaussian_Mixture_Model::Gaussian_Distribution(double data[], double mean[], double diagonal_covariance[]){
	double determinant = 1;
	double result;
	double sum = 0;

	for (int i = 0; i < dimension_data; i++){
		determinant *= diagonal_covariance[i];
		sum += (data[i] - mean[i]) * (1 / diagonal_covariance[i]) * (data[i] - mean[i]);
	}
	result = 1.0 / (pow(2 * 3.1415926535897931, dimension_data / 2.0) * sqrt(determinant)) * exp(-0.5 * sum);

	if (isnan(result) || !finite(result)){
		fprintf(stderr, "[Gaussian Distribution], [The covariance matrix is rank deficient], [result: %lf]\n", result);
	}
	return result;
}
double Gaussian_Mixture_Model::Gaussian_Distribution(double data[], double mean[], double **covariance){
	double result;
	double sum = 0;

	double **inversed_covariance = new double*[dimension_data];

	Matrix matrix;

	for (int i = 0; i < dimension_data; i++){
		inversed_covariance[i] = new double[dimension_data];
	}
	matrix.Inverse(type_covariance, dimension_data, covariance, inversed_covariance);

	for (int i = 0; i < dimension_data; i++){
		double partial_sum = 0;

		for (int j = 0; j < dimension_data; j++){
			partial_sum += (data[j] - mean[j]) * inversed_covariance[j][i];
		}
		sum += partial_sum * (data[i] - mean[i]);
	}

	for (int i = 0; i < dimension_data; i++){
		delete[] inversed_covariance[i];
	}
	delete[] inversed_covariance;

	result = 1.0 / (pow(2 * 3.1415926535897931, dimension_data / 2.0) * sqrt(matrix.Determinant(type_covariance, dimension_data, covariance))) * exp(-0.5 * sum);

	if (isnan(result) || !finite(result)){
		fprintf(stderr, "[Gaussian Distribution], [The covariance matrix is rank deficient], [result: %lf]\n", result);
	}
	return result;
}
