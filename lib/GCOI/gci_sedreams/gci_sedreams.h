#ifndef GCI_SEDREAMS_H
#define GCI_SEDREAMS_H

#include <Eigen/Dense>
#include <deque>
#include <utility>
#include <vector>

namespace Sedreams {

    std::pair<std::vector<int>, std::vector<int>> gci_sedreams(const Eigen::ArrayXd & signal, double fs, double T0mean);

    std::vector<int> findPeaks(const Eigen::ArrayXd & signal, double sign);

    std::pair<std::vector<int>, std::vector<int>> findZeroCrossings(const Eigen::ArrayXd & signal);

    double median(std::vector<double>& x);

}

#endif // GCI_SEDREAMS_H
