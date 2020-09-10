#include "util.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>

using namespace Eigen;

std::vector<std::complex<float>> Analysis::findRoots(const std::vector<float>& p)
{
    const int n = p.size();

    ArrayXd poly = Map<const ArrayXf>(p.data(), n).cast<double>();

    MatrixXd companion(n - 1, n - 1);
    companion.leftCols<1>().setZero();
    companion.bottomRows<1>() = -poly.head(n - 1);
    companion.topRightCorner(n - 2, n - 2).setIdentity();

    static EigenSolver<MatrixXd> solver;
    solver.setMaxIterations(40);
    solver.compute(companion, false);

    ArrayXcd roots = solver.eigenvalues();

    std::vector<std::complex<float>> rootsVector;
    for (int i = 0; i < roots.size(); ++i) {
        rootsVector.push_back(std::complex<float>(roots[i]));
    }
    return rootsVector;
}

