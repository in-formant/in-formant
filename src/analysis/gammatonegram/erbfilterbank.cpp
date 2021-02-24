#include "gammatonegram.h"
#include "../filter/filter.h"

Eigen::MatrixXd Analysis::erbFilterBank(const rpm::vector<double>& x, const rpm::vector<rpm::vector<std::array<double, 6>>>& filters)
{
    Eigen::MatrixXd output(filters.size(), x.size());
    for (int i = 0; i < filters.size(); ++i) {
        rpm::vector<double> y = Analysis::sosfilter(filters[i], x);
        for (int j = 0; j < y.size(); ++j) {
            output(i, j) = y[j];
        }
    }
    return output;
}
