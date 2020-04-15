#include "gci_sedreams.h"

using namespace Eigen;

std::pair<std::vector<int>, std::vector<int>> Sedreams::findZeroCrossings(const ArrayXd & signal)
{

    std::vector<int> pos, neg;

    double cur, next;

    next = signal(0);

    for (int t = 0; t < signal.size() - 1; ++t) {
        cur = next;
        next = signal(t + 1);
        
        // sign change or null
        if (cur * next <= 0) {
            // positive going or null
            if (next >= 0) {
                pos.push_back(t);
            } else {
                neg.push_back(t);
            }
        }

    }

    return std::make_pair(std::move(pos), std::move(neg));

}
