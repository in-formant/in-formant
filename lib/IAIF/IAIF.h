#ifndef IAIF_H
#define IAIF_H

#include <Eigen/Dense>

namespace IAIF
{
    Eigen::ArrayXd processFrame(const Eigen::ArrayXd & x, int nv, int ng, double d);
}

#endif // IAIF_H
