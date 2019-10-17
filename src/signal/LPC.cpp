//
// Created by rika on 12/10/2019.
//

#include "LPC.h"
#include "LPC_Frame.h"
#include "Window.h"
#include "Filter.h"

using namespace Eigen;

void LPC::shortTermAnalysis(const ArrayXd & sound, double windowDuration, double timeStep, int * numberOfFrames, double * firstTime)
{
    double duration = sound.size() * timeStep;
    *numberOfFrames = std::floor((duration - windowDuration) / timeStep) + 1;
    const double ourMidTime = (duration - timeStep) * 0.5;
    const double thyDuration = *numberOfFrames * timeStep;
    *firstTime = ourMidTime + (thyDuration - timeStep) * 0.5;
}

LPC::Frames LPC::analyse(const Eigen::ArrayXd & _sound, int predictionOrder,
                         double analysisWidth, double samplingFrequency,
                         double preEmphasisFrequency, int method)
{
    double t1, windowDuration = 2 * analysisWidth; // Gaussian window.
    int numberOfFrames, frameErrorCount = 0, iter = 0;
    int frameLength = std::round(windowDuration * samplingFrequency);

    if (frameLength > _sound.size()) {
        frameLength = _sound.size();
        windowDuration = frameLength * samplingFrequency;
    }

    LPC::shortTermAnalysis(_sound, windowDuration, samplingFrequency, &numberOfFrames, &t1);

    ArrayXd sound(_sound);
    ArrayXd sframe(frameLength);

    // Avoid rebuilding the window for every block.
    static ArrayXd window(0);
    if (window.size() != frameLength) {
        window = Window::createGaussian(frameLength);
    }

    LPC::Frames lpc;
    lpc.maxnCoefficients = predictionOrder;
    lpc.d_frames.resize(numberOfFrames);

    if (preEmphasisFrequency < samplingFrequency / 2.0) {
        Filter::preEmphasis(sound, samplingFrequency, preEmphasisFrequency);
    }

    for (int i = 0; i < numberOfFrames; ++i) {
        LPC::Frame & lpcFrame = lpc.d_frames.at(i);
        lpcFrame.nCoefficients = predictionOrder;

        // Default to zero if the frame lies out of bounds.
        sframe.setZero();
        // Check for bounds.
        int imin = std::max<int>(std::round(frameLength * (i - 0.5)), 0);
        int imax = std::min<int>(std::round(frameLength * (i + 0.5)), sound.size() - 1);
        int slen = imax - imin;
        int ismin = imin - std::round(frameLength * (i - 0.5));
        sframe.segment(ismin, slen) = sound.segment(imin, slen);

        // Remove DC and apply windowing.
        sframe -= sframe.mean();
        sframe *= window;

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

    return lpc;

}

LPC::Frames analyseAuto(const Eigen::ArrayXd & sound, int predictionOrder,
                        double analysisWidth, double samplingFrequency, double preEmphasisFrequency)
{
    return LPC::analyse(sound, predictionOrder,
                        analysisWidth, samplingFrequency,
                        preEmphasisFrequency, LPC::Method::Auto);
}

LPC::Frames analyseCovar(const Eigen::ArrayXd & sound, int predictionOrder,
                        double analysisWidth, double samplingFrequency, double preEmphasisFrequency)
{
    return LPC::analyse(sound, predictionOrder,
                        analysisWidth, samplingFrequency,
                        preEmphasisFrequency, LPC::Method::Covar);
}

LPC::Frames analyseBurg(const Eigen::ArrayXd & sound, int predictionOrder,
                        double analysisWidth, double samplingFrequency, double preEmphasisFrequency)
{
    return LPC::analyse(sound, predictionOrder,
                        analysisWidth, samplingFrequency,
                        preEmphasisFrequency, LPC::Method::Burg);
}