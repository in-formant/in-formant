//
// Created by rika on 12/10/2019.
//

#include "LPC.h"
#include "Frame/LPC_Frame.h"
#include "LPC_huber.h"
#include "../Signal/Filter.h"
#include "../Signal/Window.h"

using namespace Eigen;
using LPC::Huber::huber_s;

LPC::Frames LPC::refineRobust(
        const LPC::Frames & lpc1, const ArrayXd & _sound,
        double samplingFrequency, double analysisWidth, double preEmphasisFrequency,
        double k_stdev, int itermax, double tol, bool wantLocation)
{
    huber_s hs;

    double t1, tol_svd = 1e-6;
    double location = 0, windowDuration = 2 * analysisWidth; // Gaussian window.
    int numberOfFrames, frameErrorCount = 0, iter = 0;
    int p = lpc1.maxnCoefficients;
    int frameLength = std::round(windowDuration * samplingFrequency);

    LPC::shortTermAnalysis(_sound, windowDuration, samplingFrequency, samplingFrequency, &numberOfFrames, &t1);

    ArrayXd sound(_sound);
    ArrayXd sframe(frameLength);
    ArrayXd window(Window::createGaussian(frameLength));

    LPC::Frames lpc2 = lpc1;
    LPC::Huber::init(hs, windowDuration, p, samplingFrequency, location, wantLocation);

    if (preEmphasisFrequency < samplingFrequency / 2.0) {
        Filter::preEmphasis(sound, samplingFrequency, preEmphasisFrequency);
    }

    hs.k_stdev = k_stdev;
    hs.tol = tol;
    hs.tol_svd = tol_svd;
    hs.itermax = itermax;

    for (int i = 0; i < numberOfFrames; ++i) {
        const auto & lpc = lpc1.d_frames.at(i);
        auto & lpcto = lpc2.d_frames.at(i);

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

        // Remove DC and apply windowing.
        sframe = (sframe - sframe.mean()) * window;

        LPC::frame_huber(sframe, lpc, lpcto, hs);
    }

    return lpc2;
}
