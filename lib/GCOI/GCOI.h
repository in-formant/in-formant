#ifndef GCOI_H
#define GCOI_H

#include <Eigen/Dense>
#include <vector>

std::vector<int> findPeaks(Eigen::Ref<const Eigen::ArrayXd> x);

namespace GCOI {
    
    struct GIPair {
        int gci, goi, nextgci;
    };

    std::vector<GIPair> estimate_MultiProduct(const Eigen::ArrayXd & x, double fs, int lev);
   
    std::vector<GIPair> estimate_Sedreams(const Eigen::ArrayXd & x, double fs, double meanF0);

    double estimateOq(const std::vector<GIPair> & pairs);

}

#endif // GCOI_H
