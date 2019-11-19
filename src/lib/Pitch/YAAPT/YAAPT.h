//
// Created by rika on 10/11/2019.
//

#ifndef SPEECH_ANALYSIS_YAAPT_H
#define SPEECH_ANALYSIS_YAAPT_H

#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>
#include <array>
#include <iostream>
#include "../../Signal/Filter.h"
#include "../../Signal/Window.h"
#include "../../../analysis/Analyser.h"

namespace Eigen {
    using ArrayXb = Eigen::Array<bool, Eigen::Dynamic, 1>;

    template<class Derived, class OtherDerived>
    Array<typename Derived::Scalar, Dynamic, 1> filter(const ArrayBase<Derived> & x, const ArrayBase<OtherDerived> & cond) {
        int nx = x.size();
        int ny = cond.template cast<int>().sum();
        Array<typename Derived::Scalar, Dynamic, 1> y(ny);
        for (int i = 0, iy = 0; i < nx; ++i) {
            if (cond(i)) {
                y(iy++) = x(i);
            }
        }
        return y;
    }
}

namespace YAAPT
{
    constexpr int numFrames = analysisPitchFrameCount;

    using AudioFrames = std::array<Eigen::ArrayXd, numFrames>;

    using Tensor3d = Eigen::Tensor<double, 3>;

    using RefXd = Eigen::Ref<Eigen::ArrayXd>;
    using RefXXd = Eigen::Ref<Eigen::ArrayXXd>;
    using RefXXXd = Tensor3d &;
    using RefXcd = Eigen::Ref<Eigen::ArrayXcd>;
    using RefXXcd = Eigen::Ref<Eigen::ArrayXXcd>;
    using RefXi = Eigen::Ref<Eigen::ArrayXi>;
    using RefXb = Eigen::Ref<Eigen::ArrayXb>;

    using ConstRefXd = const Eigen::Ref<const Eigen::ArrayXd>;
    using ConstRefXXd = const Eigen::Ref<const Eigen::ArrayXXd>;
    using ConstRefXXXd = const Tensor3d &;
    using ConstRefXcd = const Eigen::Ref<const Eigen::ArrayXcd>;
    using ConstRefXXcd = const Eigen::Ref<const Eigen::ArrayXXcd>;
    using ConstRefXi = const Eigen::Ref<const Eigen::ArrayXi>;
    using ConstRefXb = const Eigen::Ref<const Eigen::ArrayXb>;

    struct Params {
        double F0min = 70;          // Minimum F0 searched (Hz)
        double F0max = 600;         // Maximum F0 searched (Hz)
        int    fftLength = 8192;    // FFT length
        int    bpOrder = 150;       // Order of bandpass filter
        double bpLow = 50;          // Low frequency of filter passband (Hz)
        double bpHigh = 1500;       // High frequency of filter passband (Hz)
        double nlferThresh1 = 0.75; // NLFER boundary for voiced/unvoiced decisions
        double nlferThresh2 = 0.1;  // Threshold for NLFER definitely unvoiced
        int    shcNumHarms = 3;     // Number of harmonics in SHC calculation
        double shcWindow = 40;      // SHC window length (Hz)
        int    shcMaxPeaks = 4;        // Maximum number of SHC peaks to be found
        double shcPWidth = 50;      // Window width in SHC peak picking (Hz)
        double shcThresh1 = 5.0;    // Threshold 1 for SHC peak picking
        double shcThresh2 = 1.25;   // Threshold 2 for SHC peak picking
        double F0double = 150;      // F0 doubling decision threshold (Hz)
        double F0half = 150;        // F0 halving decision threshold (Hz)
        double dp5_k1 = 11;         // Weight using in dynamic programming
        int    decFactor = 1;       // Factor for signal resampling
        double nccfThresh1 = 0.3;  // Threshold for considering a peak in NCCF3
        double nccfThresh2 = 0.9;   // Threshold for terminating search in NCCF
        int    nccfMaxCands = 3;    // Maximum number of candidates found
        double nccfPWidth = 5;      // Window width in NCCF peak picking
        double meritBoost = 0.20;   // Boost merit
        double meritPivot = 0.99;   // Merit assigned to unvoiced candidates in
                                    //   definitely unvoiced frames
        double meritExtra = 0.4;    // Merit assigned to extra candidates
                                    //   in reducing F0 doubling/halving errors
        int    medianValue = 7;     // Order of medial filter
        double dp_w1 = 0.15;        // DP weight factor for V-V transitions
        double dp_w2 = 0.5;         // DP weight factor for V-UV or UV-V transitions
        double dp_w3 = 0.1;         // DP weight factor for UV-UV transitions
        double dp_w4 = 0.9;         // DP weight factor for local costs

        Params()
            : F0min(70), F0max(600), fftLength(8192),
              bpOrder(150), bpLow(50), bpHigh(1500),
              nlferThresh1(0.75), nlferThresh2(0.1),
              shcNumHarms(3), shcWindow(40), shcMaxPeaks(4),
              F0double(150), F0half(150),
              dp5_k1(11),
              decFactor(1),
              nccfThresh1(0.3), nccfThresh2(0.9),
              nccfMaxCands(3), nccfPWidth(5),
              meritBoost(0.2), meritPivot(0.99), meritExtra(0.4),
              medianValue(7),
              dp_w1(0.15), dp_w2(0.5), dp_w3(0.1), dp_w4(0.9)
        {
        }
    };

    // Signal processing / util utility functions.

    void fir1(int n, ConstRefXd w, RefXd b);
    void fir2(int n, ConstRefXd f, ConstRefXd m, int grid_n, int ramp_n, RefXd b);
    void interp1(ConstRefXd x, ConstRefXd v, ConstRefXd xq, RefXd vq);
    void medfilt1(ConstRefXd x, int w, RefXd y);
    void specgram(const AudioFrames & x, int nfft, RefXXcd spec);

    // YAAPT utility functions.

    void cmp_rate(ConstRefXd phi, double fs, const Params & prm,
                  int maxCands, int lagMin, int lagMax,
                  RefXd pitch, RefXd merit);

    void crs_corr(ConstRefXd data, int lagMin, int lagMax, RefXd phi);

    void dynamic5(ConstRefXXd pitchArray, ConstRefXXd meritArray,
                  double k1, const Params & prm, RefXd finalPitch);

    void dynamic(ConstRefXXd pitchArray, ConstRefXXd meritArray,
                 ConstRefXd energy, const Params & prm,
                 RefXd finalPitch);

    void nlfer(const AudioFrames & data, double fs, const Params & prm,
               RefXd energy, RefXb vUvEnergy);

    void nonlinear(const AudioFrames & A, double fs, const Params & prm,
                   AudioFrames & B, AudioFrames & C,
                   AudioFrames & D, double & newFs);

    void path1(ConstRefXXd local, ConstRefXXXd trans, RefXi path);

    void peaks(ConstRefXd data, double delta, int maxPeaks, const Params & prm,
               RefXd pitch, RefXd merit);

    void refine(ConstRefXXd tPitch1, ConstRefXXd tMerit1,
                ConstRefXXd tPitch2, ConstRefXXd tMerit2,
                ConstRefXd sPitch, ConstRefXd energy, ConstRefXb vUvEnergy, const Params & prm,
                RefXXd pitch, RefXXd merit);

    void ptch_fix(RefXd pitch, int pHalf, int pDouble);

    void spec_trk(const AudioFrames & data, double fs,
                  ConstRefXb vUvEnergy, const Params & prm,
                  RefXd sPitch, RefXd vUvSPitch, double & pAvg, double & pStd);

    void spec_trk2(const AudioFrames & data, double fs,
                   ConstRefXb vUvEnergy, const Params & prm,
                   RefXd sPitch, RefXd vUvSPitch, double & pAvg, double & pStd);

    void tm_trk(const AudioFrames & data, double fs, RefXd sPitch,
                double pStd, double pAvg, const Params & prm,
                Eigen::ArrayXXd & pitch, Eigen::ArrayXXd & merit);

    // YAAPT tracking functions!!
    struct Result {
        Eigen::ArrayXd pitch;  // Final pitch track in Hz. Unvoiced frames are zeroed.
        int numFrames;         // Total number of calculated frames.
        double framePeriod;    // Frame period (1/rate) of output pitch track in ms.
    };

    void getF0_slow(const AudioFrames & data, double fs,
                    Result & res, const Params & prm = Params());

    void getF0_fast(const AudioFrames & data, double fs,
                    Result & res, const Params & prm = Params());

    void getF0_fastest(const AudioFrames & data, double fs,
                       Result & res, const Params & prm = Params());

}

#endif //SPEECH_ANALYSIS_YAAPT_H
