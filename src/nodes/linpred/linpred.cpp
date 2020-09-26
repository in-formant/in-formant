#include "linpred.h"
#include <iostream>

using namespace Nodes;

constexpr NodeIOType outTypes[] = {
    kNodeIoTypeIIRFilter,
    kNodeIoTypeAudioSpec,
};

LinPred::LinPred(Analysis::LinpredSolver *solver, int order)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = outTypes,
        }),
      mSolver(solver),
      mFFT(512, FFTW_R2HC),
      mOrder(order)
{
}

void LinPred::setOrder(int order)
{
    mOrder = order;
}

inline float G(float x, int L, float alpha)
{
    const int N = L - 1;
    const float k = (x - N / 2.0f) / (2 * L * alpha);
    return expf(-(k * k));
}

static void calcGaussian(std::vector<float>& win, float alpha)
{
    const int L = win.size();
    
    float Gmh = G(-0.5f, L, alpha);
    float GmhpLpGmhmL = G(-0.5f + L, L, alpha) - G(-0.5f - L, L, alpha);

    for (int n = 0; n < L; ++n) {
        win[n] = G(n, L, alpha) - (Gmh * (G(n + L, L, alpha) + G(n - L, L, alpha))) / GmhpLpGmhmL;
    }
}

void LinPred::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::IIRFilter>();
    auto outSpec = outputs[1]->as<IO::AudioSpec>();

    int sampleRate = in->getSampleRate();

    int inLength = in->getLength();

    static std::vector<float> window;
    if (window.size() != inLength) {
        constexpr float alpha = 0.2;
        window.resize(inLength);
        calcGaussian(window, alpha);
    }

    auto inData = std::make_unique<float[]>(inLength);

    for (int i = 0; i < inLength; ++i) {
        inData[i] = in->getConstData()[i] * window[i];
    }

    float gain;
    std::vector<float> lpc = mSolver->solve(
            inData.get(),
            inLength,
            mOrder,
            &gain);

    out->setSampleRate(sampleRate);

    out->setFFOrder(1);
    out->getFFData()[0] = 1e5 * gain;
    
    out->setFBOrder(lpc.size());
    std::copy(lpc.begin(), lpc.end(), out->getFBData());

    // Filter frequency response.

    int nfft = mFFT.getLength();
    int outLength = nfft / 2 + 1;

    mFFT.data(0) = 1.0f;

    for (int i = 0; i < lpc.size(); ++i)
        mFFT.data(i + 1) = lpc[i];

    for (int i = lpc.size() + 1; i < nfft; ++i)
        mFFT.data(i) = 0.0;

    mFFT.compute();

    outSpec->setSampleRate(sampleRate);
    outSpec->setLength(outLength);

    for (int i = 0; i < outLength; ++i) {
        outSpec->getData()[i] = out->getFFConstData()[0]
                                / (mFFT.data(i) * mFFT.data(i)
                                        + mFFT.data(nfft - 1 - i) * mFFT.data(nfft - 1 - i));
    }
}
