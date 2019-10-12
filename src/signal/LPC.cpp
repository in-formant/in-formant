//
// Created by rika on 12/10/2019.
//

#include "LPC.h"

using namespace Eigen;

void LPC::shortTermAnalysis(const ArrayXd & sound, double windowDuration, double timeStep, int * numberOfFrames, double * firstTime)
{
    double duration = sound.size() * timeStep;
    *numberOfFrames = std::floor((duration - windowDuration) / timeStep) + 1;
    const double ourMidTime = (duration - timeStep) * 0.5;
    const double thyDuration = *numberOfFrames * timeStep;
    *firstTime = ourMidTime + (thyDuration - timeStep) * 0.5;
}