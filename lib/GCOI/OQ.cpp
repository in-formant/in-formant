#include "GCOI.h"
#include <iostream>

double GCOI::estimateOq(const std::vector<GIPair> & pairs)
{
    
    double Oq = 0.0;

    for (const auto & pair : pairs) {

        const double oneOq = (double) (pair.nextgci - pair.goi) / (double) (pair.nextgci - pair.gci);

        Oq += oneOq;
    }

    Oq /= (double) pairs.size();

    return Oq;

}
