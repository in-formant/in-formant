//
// Created by clo on 08/11/2019.
//

#include "Formant.h"

using namespace Eigen;

void Formant::sort(Frame & frm)
{
    std::sort(frm.formant.begin(), frm.formant.end(),
               [](const auto & x, const auto & y) {
                    return x.frequency < y.frequency;
                });
}

void Formant::frameFromRoots(
        const Eigen::ArrayXcd & r, Frame & frm,
        double samplingFrequency, double margin)
{
    frm.formant.clear();
    frm.formant.reserve(r.size());

    double fLow = margin, fHigh = samplingFrequency / 2.0 - margin;

    for (const auto & v : r) {
        if (v.imag() < 0) {
            continue;
        }

        double f = std::abs(std::arg(v)) * samplingFrequency / (2.0 * M_PI);
        if (f >= fLow && f <= fHigh) {
            double b = -std::log10(std::abs(v)) * samplingFrequency / M_PI;

            frm.formant.push_back({f, b});
        }
    }

    frm.nFormants = frm.formant.size();
    ::Formant::sort(frm);
}