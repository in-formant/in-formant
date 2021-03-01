#ifndef DEEP_FORMANTS_H
#define DEEP_FORMANTS_H

#include <Eigen/Dense>

#define MAX_AMPLITUDE_16BIT (32767.0)

template<typename Derived>
extern Eigen::ArrayXd build_feature_row(const Eigen::ArrayBase<Derived>& x);

#endif // DEEP_FORMANTS_H
