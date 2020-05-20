#include "../Analyser.h"
#include "IAIF/IAIF.h"

using namespace Eigen;

void Analyser::applyInverseFilter()
{
    speechSignal = x;
    sourceSignal = IAIF::processFrame(x, 22, 3, 0.99);
}
