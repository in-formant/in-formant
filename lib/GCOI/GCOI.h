#ifndef GCOI_H
#define GCOI_H

#include <Eigen/Dense>
#include "../rpmalloc.h"

rpm::vector<int> findPeaks(Eigen::Ref<const Eigen::ArrayXd> x);

namespace GCOI {
    
    struct GIPair {
        int gci, goi, nextgci;
    };

    rpm::vector<GIPair> estimate_MultiProduct(const Eigen::ArrayXd & x, double fs, int lev);
   
    rpm::vector<GIPair> estimate_Sedreams(const Eigen::ArrayXd & x, double fs, double meanF0);

    double estimateOq(const rpm::vector<GIPair> & pairs);

}

#endif // GCOI_H
