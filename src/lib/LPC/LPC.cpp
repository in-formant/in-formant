//
// Created by rika on 12/10/2019.
//

#include <iostream>
#include "LPC.h"
#include "Frame/LPC_Frame.h"
#include "../Signal/Window.h"
#include "../Signal/Filter.h"

using namespace Eigen;

void LPC::shortTermAnalysis(const ArrayXd & sound, double windowDuration, double samplingFrequency, double timeStep, int * numberOfFrames, double * firstTime)
{
    double duration = sound.size() / samplingFrequency;
    *numberOfFrames = std::floor((duration - windowDuration) / timeStep) + 1;
    const double ourMidTime = (duration - timeStep) * 0.5;
    const double thyDuration = *numberOfFrames * timeStep;
    *firstTime = ourMidTime + (thyDuration - timeStep) * 0.5;
}

LPC::Frames LPC::analyse(const Eigen::ArrayXd & _sound, int predictionOrder,
                         double samplingFrequency,
                         int method)
{
    int frameErrorCount = 0, iter = 0;
    int numberOfFrames = 1;
    int frameLength = _sound.size();
    double windowDuration = frameLength / samplingFrequency;

    ArrayXd sound(_sound);
    ArrayXd sframe(frameLength);

    LPC::Frames lpc;
    lpc.maxnCoefficients = predictionOrder;
    lpc.d_frames.resize(numberOfFrames);

    for (int i = 0; i < numberOfFrames; ++i) {
        LPC::Frame & lpcFrame = lpc.d_frames.at(i);
        lpcFrame.nCoefficients = predictionOrder;

        // Copy sound frame.
        int frameInd = 0;
        int soundInd = std::round(frameLength * (i - 0.5));

        for (; frameInd < frameLength; ++frameInd, ++soundInd) {
            if (soundInd < 0 || soundInd >= sound.size()) {
                sframe(frameInd) = 0.0;
            }
            else {
                sframe(frameInd) = sound(soundInd);
            }
        }

        // Remove DC.
        sframe = (sframe - sframe.mean());

        switch (method) {
        case Auto:
            if (!LPC::frame_auto(sframe, lpcFrame))
                frameErrorCount++;
            break;
        case Covar:
            if (!LPC::frame_covar(sframe, lpcFrame))
                frameErrorCount++;
            break;
        case Burg:
            if (!LPC::frame_burg(sframe, lpcFrame))
                frameErrorCount++;
            break;
        default:
            frameErrorCount++;
        }
    }

    if (frameErrorCount > 0) {
        std::cout << "LPC error: (" << frameErrorCount << " out of " << numberOfFrames << ")" << std::endl;
    }

    return lpc;

}

LPC::Frames LPC::analyseAuto(const Eigen::ArrayXd & sound, int predictionOrder, double samplingFrequency)
{
    return LPC::analyse(sound, predictionOrder,
                        samplingFrequency,
                        LPC::Method::Auto);
}

LPC::Frames LPC::analyseCovar(const Eigen::ArrayXd & sound, int predictionOrder, double samplingFrequency)
{
    return LPC::analyse(sound, predictionOrder,
                        samplingFrequency,
                        LPC::Method::Covar);
}

LPC::Frames LPC::analyseBurg(const Eigen::ArrayXd & sound, int predictionOrder, double samplingFrequency)
{
    return LPC::analyse(sound, predictionOrder,
                        samplingFrequency,
                        LPC::Method::Burg);
}