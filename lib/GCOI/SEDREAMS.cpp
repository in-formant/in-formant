#include "gci_sedreams/gci_sedreams.h"
#include "GCOI.h"

using namespace Eigen;

rpm::vector<GCOI::GIPair> GCOI::estimate_Sedreams(const ArrayXd & x, const double fs, const double meanF0) {
    
    const double meanT0 = 1.0 / meanF0;

    const auto & [ gci, goi ] = Sedreams::gci_sedreams(x, fs, meanT0);

    rpm::vector<GIPair> pairs;

    for (int i = 0; i < signed(gci.size()) - 1; ++i) {
        GIPair pair;
        pair.gci = gci[i];
        pair.nextgci = gci[i + 1];

        for (int j = 0; j < signed(goi.size()); ++j) {
            int to = goi[i];

            if (pair.gci < to && to < pair.nextgci) {
                pair.goi = to;
                pairs.push_back(pair);
            }
        }
    }
    
    return pairs;

}
