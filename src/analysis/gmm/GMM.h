#ifndef GMM_H
#define GMM_H

#include <string>

using namespace std;

class Gaussian_Mixture_Model{
private:
	string type_covariance;

	int dimension_data;
	int number_gaussian_components;
public:
	double *weight;

	double **mean;

	double **diagonal_covariance;
	double ***covariance;

	Gaussian_Mixture_Model(string type_covariance, int dimension_data, int number_gaussian_components);
	~Gaussian_Mixture_Model();

	void Initialize(int number_data, double **data);
	void Load_Parameter(string path);
	void Save_Parameter(string path);

	int Classify(double data[]);

	double Calculate_Likelihood(double data[]);
	double Calculate_Likelihood(double data[], double gaussian_distribution[]);
	double Expectaion_Maximization(int number_data, double **data);
	double Gaussian_Distribution(double data[], int component_index);
	double Gaussian_Distribution(double data[], double mean[], double diagonal_covariance[]);
	double Gaussian_Distribution(double data[], double mean[], double **covariance);
};

#endif
