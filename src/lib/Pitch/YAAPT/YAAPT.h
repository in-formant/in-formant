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
}

namespace YAAPT
{
    constexpr int numFrames = analysisAudioFrames;

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
        double nccfThresh1 = 0.25;  // Threshold for considering a peak in NCCF
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
              nccfThresh1(0.25), nccfThresh2(0.9),
              nccfMaxCands(3), nccfPWidth(5),
              meritBoost(0.2), meritPivot(0.99), meritExtra(0.4),
              medianValue(7),
              dp_w1(0.15), dp_w2(0.5), dp_w3(0.1), dp_w4(0.9)
        {
        }
    };

    // Signal processing / util utility functions.

    void fir1(int n, const Eigen::ArrayXd & w, Eigen::ArrayXd & b);
    void fir2(int n, const Eigen::ArrayXd & f, const Eigen::ArrayXd & m,
              int grid_n, int ramp_n, Eigen::ArrayXd & b);
    void interp1(const Eigen::ArrayXd & x, const Eigen::ArrayXd & v,
                 const Eigen::ArrayXd & xq, Eigen::ArrayXd & vq);
    void medfilt1(const Eigen::ArrayXd & x, int w, Eigen::ArrayXd & y);
    void specgram(const std::array<Eigen::ArrayXd, numFrames> & x, int nfft,
                  Eigen::ArrayXXcd & spec);

    // YAAPT utility functions.

    void cmp_rate(const Eigen::ArrayXd & phi, double fs, const Params & prm,
                  int maxCands, int lagMin, int lagMax,
                  Eigen::ArrayXd & pitch, Eigen::ArrayXd & merit);

    void crs_corr(const Eigen::ArrayXd & data, int lagMin, int lagMax,
                  Eigen::ArrayXd & phi);

    void dynamic5(const Eigen::ArrayXXd & pitchArray, const Eigen::ArrayXXd & meritArray,
                  double k1, const Params & prm,
                  Eigen::ArrayXd & finalPitch);

    void dynamic(const Eigen::ArrayXXd & pitchArray, const Eigen::ArrayXXd & meritArray,
                 const Eigen::ArrayXd & energy, const Params & prm,
                  Eigen::ArrayXd & finalPitch);

    void nlfer(const std::array<Eigen::ArrayXd, numFrames> & data, double fs, const Params & prm,
               Eigen::ArrayXd & energy, Eigen::ArrayXb & vUvEnergy);

    void nonlinear(const std::array<Eigen::ArrayXd, numFrames> & A, double fs, const Params & prm,
                   std::array<Eigen::ArrayXd, numFrames> & B, std::array<Eigen::ArrayXd, numFrames> & C,
                   std::array<Eigen::ArrayXd, numFrames> & D, double & newFs);

    void path1(const Eigen::ArrayXXd & local, const Eigen::Tensor<double, 3> & trans, Eigen::ArrayXi & path);

    void peaks(const Eigen::ArrayXd & data, double delta, int maxPeaks, const Params & prm,
               Eigen::ArrayXd & pitch, Eigen::ArrayXd & merit);

    void refine(const Eigen::ArrayXXd & tPitch1, const Eigen::ArrayXXd & tMerit1,
                const Eigen::ArrayXXd & tPitch2, const Eigen::ArrayXXd & tMerit2,
                const Eigen::ArrayXd & sPitch, const Eigen::ArrayXd & energy,
                const Eigen::ArrayXb & vUvEnergy, const Params & prm,
                Eigen::ArrayXXd & pitch, Eigen::ArrayXXd & merit);

    void spec_trk(const std::array<Eigen::ArrayXd, numFrames> & data, double fs,
                  const Eigen::ArrayXb & vUvEnergy, const Params & prm,
                  Eigen::ArrayXd & sPitch, Eigen::ArrayXd & vUvSPitch, double & pAvg, double & pStd);

    void spec_trk2(const std::array<Eigen::ArrayXd, numFrames> & data, double fs,
                   const Eigen::ArrayXb & vUvEnergy, const Params & prm,
                   Eigen::ArrayXd & sPitch, Eigen::ArrayXd & vUvSPitch, double & pAvg, double & pStd);

    void tm_trk(const std::array<Eigen::ArrayXd, numFrames> & data, double fs, Eigen::ArrayXd & sPitch,
                double pStd, double pAvg, const Params & prm,
                Eigen::ArrayXXd & pitch, Eigen::ArrayXXd & merit);

    // YAAPT tracking functions!!
    struct Result {
        Eigen::ArrayXd pitch;  // Final pitch track in Hz. Unvoiced frames are zeroed.
        int numFrames;         // Total number of calculated frames.
        double framePeriod;    // Frame period (1/rate) of output pitch track in ms.
    };

    void getF0_slow(const std::array<Eigen::ArrayXd, numFrames> & data, double fs,
                    Result & res, const Params & prm = Params());

    void getF0_fast(const std::array<Eigen::ArrayXd, numFrames> & data, double fs,
                    Result & res, const Params & prm = Params());

    void getF0_fastest(const Eigen::ArrayXd & data, double fs,
                       Result & res, const Params & prm = Params());

}

#endif //SPEECH_ANALYSIS_YAAPT_H
