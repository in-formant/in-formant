#include "../Analyser.h"
#include "IAIF/IAIF.h"
#include "GCOI/GCOI.h"
#include "Signal/Window.h"

using namespace Eigen;

void Analyser::applyInverseFilter()
{
    const int nv = 2 * std::round(fs / 2000) + 4;
    const int ng = 2 * std::round(fs / 4000);
    const double d = 0.99;

    double meanPitch = lastPitchFrame;
    if (meanPitch <= 50 || meanPitch > 2500) {
        meanPitch = 180;
    }

    rpm::vector<GCOI::GIPair> gis = GCOI::estimate_Sedreams(x, fs, meanPitch);

    speechSignal = x;
    sourceSignal.setZero(x.size());

    for (int i = 0; i < signed(gis.size()); ++i) {
        const auto& pair = gis.at(i);
        int T0 = pair.nextgci - pair.gci;
        int start = pair.gci - T0 / 2;
        int len = T0;
        
        if (start < 0) {
            start = 0;
        }
        if (start + len > x.size() - 1) {
            len = x.size() - start - 1;
        }

        ArrayXd xseg = speechSignal.segment(start, len);

        sourceSignal.segment(start, len) += IAIF::processFrame(xseg, nv, ng, d);
    }

    sourceSignal = IAIF::processFrame(x, nv, ng, d);
}
