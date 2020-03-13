#ifndef YIN_H
#define YIN_H

#include <Eigen/Dense>
#include <utility>

namespace YIN
{
    Eigen::ArrayXd autocorrelation(Eigen::Ref<const Eigen::ArrayXd> x);

    Eigen::ArrayXd difference(const Eigen::ArrayXd & x);

    Eigen::ArrayXd cumulative_mean_normalized_difference(const Eigen::ArrayXd & x);

    int absolute_threshold(const Eigen::ArrayXd & x, double threshold);

    std::pair<double, double> parabolic_interpolation(Eigen::Ref<const Eigen::ArrayXd> array, int x);
}

#endif // YIN_H
