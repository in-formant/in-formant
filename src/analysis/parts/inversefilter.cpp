#include "../Analyser.h"
#include "IAIF/IAIF.h"

using namespace Eigen;

void Analyser::applyInverseFilter()
{
    speechSignal = x;
    sourceSignal = IAIF::processFrame(x, 20, 3, 0.98);
}
