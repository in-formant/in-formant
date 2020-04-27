#ifndef CREPE_H
#define CREPE_H

#include <Eigen/Dense>

enum { MODEL_TINY = 0, MODEL_SMALL, MODEL_MEDIUM, MODEL_LARGE, MODEL_FULL };

Eigen::ArrayXd get_activation(const Eigen::ArrayXd& x);

double to_local_average_cents(const Eigen::ArrayXd& salience, int center);

double cents_to_hertz(double cents);

#endif // CREPE_H
