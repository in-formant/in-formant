#ifndef GCI_SEDREAMS_H
#define GCI_SEDREAMS_H

#include <Eigen/Dense>
#include <utility>
#include "../../rpmalloc.h"

namespace Sedreams {

    std::pair<rpm::vector<int>, rpm::vector<int>> gci_sedreams(const Eigen::ArrayXd & signal, double fs, double T0mean);

    rpm::vector<int> findPeaks(const Eigen::ArrayXd & signal, double sign);

    std::pair<rpm::vector<int>, rpm::vector<int>> findZeroCrossings(const Eigen::ArrayXd & signal);

    double median(rpm::vector<double>& x);

}

#endif // GCI_SEDREAMS_H
