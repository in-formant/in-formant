#include "GCOI.h"
#include <iostream>

using namespace Eigen;

rpm::vector<GCOI::GIPair> GCOI::estimate_MultiProduct(const ArrayXd & _x, double fs, int lev)
{
    // Resize to a multiple of (2^lev).
    int dataLen = (_x.size() / (2 << (lev + 1))) * (2 << (lev + 1));

    ArrayXd a = _x.head(dataLen);
    ArrayXd p = ArrayXd::Ones(dataLen);

    for (int i = 1; i < lev; ++i) {
        int div = 2 << (i - 1);
        int newWidth = dataLen / div;

        // Perform the FLWT.
        
        ArrayXd d(newWidth);
        ArrayXd acur(newWidth);
        for (int j = 0; j < newWidth; ++j) {
            d(j) = a(2 * j + 1) - a(2 * j);
            acur(j) = a(2 * j) + d(j) / 2;

            for (int k = 0; k < div; ++k) {
                p(div * j + k) *= acur(j);
            }
        }
        a = std::move(acur);
    }

    rpm::vector<int> gcis = findPeaks(-p);
    rpm::vector<GIPair> pairs;

    double pitch = 0;

    for (int k = 0; k < signed(gcis.size()) - 1; ++k) {
        GIPair pair;
    
        pair.gci = gcis[k];
        pair.nextgci = gcis[k + 1];
        
        int t1 = 0.8 * gcis[k] + 0.2 * gcis[k + 1];
        int t2 = gcis[k + 1];
        
        p(seq(t1, t2)).maxCoeff(&pair.goi);
        pair.goi += pair.gci;

        pairs.push_back(std::move(pair));

        pitch += fs / (2 * (pair.nextgci - pair.gci));
    }

    pitch /= signed(gcis.size()) - 1;

    return pairs;

}
