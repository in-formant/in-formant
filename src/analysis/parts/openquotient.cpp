#include "../Analyser.h"
#include "GCOI/GCOI.h"

void Analyser::analyseOq()
{
    if (lastPitchFrame != 0.0) {
        std::vector<GCOI::GIPair> giPairs = GCOI::estimate_MultiProduct(x, fs, 3);

        lastOqFrame = GCOI::estimateOq(giPairs);
    }
    else {
        lastOqFrame = 0.0;
    }
}
