//
// Created by rika on 10/11/2019.
//

#include "YAAPT.h"

void YAAPT::nonlinear(
        const AudioFrames & A, double fs, const Params & prm,
        AudioFrames & B, AudioFrames & C,
        AudioFrames & D, double & newFs)
{
    using namespace Eigen;

    constexpr double fsMin = 1000; // Do not decimate if fs less than this.

    // Parameters for filtering original signal, with a broader band.
    int filterOrder = prm.bpOrder;
    double F_hp = prm.bpLow;
    double F_lp = prm.bpHigh;

    double decFactor = (fs > fsMin) ? prm.decFactor : 1;

    int lenDataA = A.size();
    int lenData_dec = std::floor((lenDataA + decFactor - 1) / decFactor);

    // Create the bandpass filter.
    // Filter F1.
    ArrayXd w(2);
    w << F_hp / (fs / 2.0), F_lp / (fs / 2.0);
    ArrayXd b_F1(filterOrder + 1);
    fir1(filterOrder, w, b_F1);

    ArrayXd tempData;

    for (int i = 0; i < numFrames; ++i) {
        // Filtering the original data with the bandpass filter.
        // Original signal filtered with F1
        Filter::apply(b_F1, A[i], tempData);
        // B = tempData(1:dec_factor:lenDataA)
        B[i] = tempData(seq(0, last, decFactor));

        // Create nonlinear version of signal
        C[i] = A[i].square();

        // Nonlinear version filtered with F1
        Filter::apply(b_F1, C[i], tempData);
        // D = tempData(1:dec_factor:lenDataA)
        D[i] = tempData(seq(0, last, decFactor));
    }

    newFs = fs / decFactor;

}
