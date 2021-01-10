#ifndef KMEANS_H
#define KMEANS_H

class KMeans{
private:
	int dimension_data;
	int number_clusters;
public:
	double **centroid;

	KMeans(int dimension_data, int number_clusters);
	~KMeans();

	// forgy method
	void Initialize(int number_data, double **data);

	int Classify(double data[]);

	double Cluster(int number_data, double **data);
};

#endif
