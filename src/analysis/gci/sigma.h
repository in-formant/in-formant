#ifndef ANALYSIS_SIGMA_H
#define ANALYSIS_SIGMA_H

#include <vector>
#include <tuple>

namespace SIGMA {

    std::vector<double> analyse(const std::vector<double>& lx, double fs);

    // [tew, sew, y, toff]
    std::tuple<std::vector<double>, std::vector<double>, std::vector<double>, int>
        xewgrdel(const std::vector<double>& u, double fs, double gwlen, double fwlen);

}

#endif // ANALYSIS_SIGMA_H
