#ifndef ANALYSIS_SIGMA_H
#define ANALYSIS_SIGMA_H

#include "rpcxx.h"
#include <tuple>

namespace SIGMA {

    rpm::vector<double> analyse(const rpm::vector<double>& lx, double fs);

    // [tew, sew, y, toff]
    std::tuple<rpm::vector<double>, rpm::vector<double>, rpm::vector<double>, int>
        xewgrdel(const rpm::vector<double>& u, double fs, double gwlen, double fwlen);

}

#endif // ANALYSIS_SIGMA_H
