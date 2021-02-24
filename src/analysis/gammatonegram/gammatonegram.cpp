#include "gammatonegram.h"

Eigen::VectorXd Analysis::gammatoneGram(const rpm::vector<double>& x, double fs, int N, double lowFreq)
{
    auto filters = makeErbFilters(fs, N, lowFreq);
    std::reverse(filters.begin(), filters.end());
    auto xf = erbFilterBank(x, filters);
    xf = xf.cwiseProduct(xf).eval();

    return xf.rowwise().mean().cwiseSqrt();
}
