//
// Created by rika on 11/11/2019.
//

#include <fftw3.h>
#include "YAAPT.h"
#include "../../Signal/Filter.h"
#include "../../Signal/Window.h"

using namespace Eigen;

void YAAPT::spec_trk(
        const ArrayXd & data, double fs, const ArrayXb & vUvEnergy, const Params & prm,
        ArrayXd & sPitch, ArrayXd & vUvSPitch, double & pAvg, double & pStd)
{
    int frameSize = std::floor(prm.frameLength * fs / 1000.0);
    int frameJump = std::floor(prm.frameSpace * fs / 1000.0);
    int overlap = frameSize - frameJump;
    int numFrames = std::floor((data.size() - overlap) / frameJump);

    int maxPeaks = prm.shcMaxPeaks;
    int nfft = prm.fftLength;
    double delta = fs / static_cast<double>(nfft);

    int windowLength = std::floor(prm.shcWindow / delta);
    if (windowLength % 2 == 0) {
        windowLength++;
    }

    int halfWindowLength = std::floor(prm.shcWindow / (delta * 2));
    int maxSHC = std::floor((prm.F0max + prm.shcPWidth * 2) / delta);
    int minSHC = std::ceil(prm.F0min / delta);
    int numHarmonics = prm.shcNumHarms;

    // Initialisation
    ArrayXXd candsPitch = ArrayXXd::Zero(maxPeaks, numFrames);
    ArrayXXd candsMerit = ArrayXXd::Ones(maxPeaks, numFrames);
    // Zero padding.
    ArrayXd dataPad(data.size() + frameSize);
    dataPad.head(data.size()) = data;
    dataPad.tail(frameSize).setZero();

    // Compute SHC for voiced frames.
    ArrayXd window = Window::createKaiser(frameSize);
    ArrayXd shc = ArrayXd::Zero(maxSHC);

    ArrayXXi winix(numHarmonics + 1, windowLength);
    ArrayXXi rowix(numHarmonics + 1, windowLength);

    for (int i = 0; i < numHarmonics + 1; ++i) {
        winix.row(i).setLinSpaced(windowLength, 0, windowLength - 1);
    }
    for (int j = 0; j < windowLength; ++j) {
        rowix.col(j).setLinSpaced(numHarmonics + 1, 0, numHarmonics);
    }

    int magnit1_len = std::floor((numHarmonics + 2) * prm.F0max / delta) + windowLength;

    for (int frame = 0; frame < numFrames; ++frame) {
        if (vUvEnergy(frame)) {
            ArrayXd signal(nfft);
            ArrayXcd fftFrame(frameSize);
            fftw_plan plan;

            plan = fftw_plan_dft_r2c_1d(frameSize, signal.data(), (fftw_complex *) fftFrame.data(), 0);
            signal.head(nfft - frameSize).setZero();
            signal.tail(frameSize) = data.segment(frame * frameJump, frameSize) * window;

            fftw_execute(plan);
            fftw_destroy_plan(plan);

            ArrayXd magnit(halfWindowLength + nfft);
            magnit.head(halfWindowLength).setZero();
            magnit.tail(nfft) = fftFrame.abs();

            ArrayXd magnit1 = magnit.head(magnit1_len);
            magnit1 /= magnit1.maxCoeff();

            // Compute SHC
            ArrayXXi ix = winix + minSHC * rowix;
            for (int k = minSHC; k <= maxSHC; ++k) {
                ArrayXXd mat(numHarmonics + 1, windowLength);
                for (int idx = 0; idx < numHarmonics + 1; ++idx) {
                    mat.row(idx) = magnit1(ix.row(idx));
                }
                shc(k - 1) = mat.colwise().prod().sum();
                ix += rowix;
            }

            double a = shc.maxCoeff();
            if (a > 1) {
                shc /= a;
            }

            ArrayXd pitchFrame, meritFrame;
            peaks(shc, delta, maxPeaks, prm, pitchFrame, meritFrame);

            candsPitch.col(frame) = pitchFrame;
            candsMerit.col(frame) = meritFrame;
        }
        else {
            // If energy is low, let frame be considered unvoiced.
            candsPitch.col(frame).setZero();
            candsMerit.col(frame).setOnes();
        }
    }

    // Extract the pitch candidates of voiced frames for the future pitch selection.
    sPitch = candsPitch.row(0);
    ArrayXb idx_voiced = (sPitch > 0);
    ArrayXXd vCandsPitch = candsPitch(all, idx_voiced);
    ArrayXXd vCandsMerit = candsMerit(all, idx_voiced);
    int numVCands = idx_voiced.cast<int>().sum();

    // Average and STD of the first choice candidates
    double avgVoiced = vCandsPitch.row(0).mean();
    double stdVoiced = std::sqrt((vCandsPitch.row(0) - vCandsPitch.row(0).mean()).square().sum() / (numFrames - 1));

    // Weight the deltas, so that higher merit candidates are considered
    // more favorably.
    ArrayXXd delta1 = (vCandsPitch - 0.8 * avgVoiced).abs() * (3 - vCandsMerit);

    // Interpolation of the weighted candidates
    ArrayXd vR(numVCands);
    ArrayXi idx(numVCands);
    for (int n = 0; n < numVCands; ++n) {
        vR(n) = delta1.col(n).minCoeff(&idx(n));
    }
    ArrayXd vPeak_minmrt(numVCands);
    ArrayXd vMerit_minmrt(numVCands);
    for (int n = 0; n < numVCands; ++n) {
        vPeak_minmrt(n) = vCandsPitch(idx(n), n);
        vMerit_minmrt(n) = vCandsMerit(idx(n), n);
    }
    if (true) { // To restrict tmp's scope.
        ArrayXd tmp;
        medfilt1(vPeak_minmrt, std::max(1, prm.medianValue - 2), tmp);
        vPeak_minmrt = tmp;
    }

    // Replace the lowest merit candidates by the median smoothed ones
    // computed from highest merit peaks above.
    for (int n = 0; n < numVCands; ++n) {
        vCandsPitch(idx(n), n) = vPeak_minmrt(n);
        vCandsMerit(idx(n), n) = vMerit_minmrt(n);
    }

    // Use dynamic programming to find best overall path among pitch candidates
    // Dynamic weight for transition costs balance between local and transition.
    double weightTrans = prm.dp5_k1 * stdVoiced / avgVoiced;

    ArrayXd vPitch;
    if (numVCands > 2) {
        dynamic5(vCandsPitch, vCandsMerit, weightTrans, prm, vPitch);

        ArrayXd tmp;
        medfilt1(vPitch, std::max(1, prm.medianValue - 2), tmp);
        vPitch = tmp;
    }
    else {
        if (numVCands > 0) {
            vPitch.resize(numVCands);
            for (int i = 0; i < numVCands; ++i) {
                vPitch(i) = 150;
            }
        }
        else {
            vPitch.resize(1);
            vPitch(0) = 150;
            idx_voiced.resize(sPitch.size());
            idx_voiced(0) = true;
        }
    }

    // Computing stats from the voiced frames.
    pAvg = vPitch.mean();
    pStd = std::sqrt((vPitch.row(0) - vPitch.row(0).mean()).square().sum() / (numVCands - 1));

    // Stretching out the smoothed pitch track.
    for (int i = 0, iv = 0; i < sPitch.size(); ++i) {
        if (idx_voiced(i)) {
            sPitch(i) = vPitch(iv++);
        }
    }

    // Interpolating through unvoiced frames.
    if (sPitch(0) < pAvg / 2.0)
        sPitch(0) = pAvg;
    if (sPitch(numFrames - 1) < pAvg / 2.0)
        sPitch(numFrames - 1) = pAvg;

    int spn = (sPitch > 0).count();
    ArrayXd sp_x(spn), sp_v(spn);
    ArrayXd sp_xq(numFrames);

    for (int ispx = 0, i = 0; i < numFrames; ++i) {
        if (sPitch(i) > 0) {
            sp_x(ispx) = i;
            sp_v(ispx) = sPitch(i);
            ispx++;
        }
    }

    std::iota(sp_xq.begin(), sp_xq.end(), 0);

    if (true) {
        ArrayXd tmp;
        interp1(sp_x, sp_v, sp_xq, tmp);
        sPitch = tmp;
    }

    constexpr int FILTER_ORDER = 3;
    ArrayXd b = ArrayXd::Ones(FILTER_ORDER) / static_cast<double>(FILTER_ORDER);

    if (true) {
        ArrayXd tmp;
        Filter::apply(b, sPitch, tmp);
        sPitch = tmp;
    }

    // Above messes up first few values of SPitch. simple fix.
    if (sPitch.size() >= 4) {
        sPitch(0) = sPitch(2);
        sPitch(1) = sPitch(3);
    }

    // Create pitch track with voiced/unvoiced decision
    vUvSPitch = vUvEnergy.select(sPitch, 0);

}
