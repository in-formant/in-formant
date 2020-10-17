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
            .outputCount = 2,
            .outputs = outTypes,
        }),
      mSolver(solver),
      mFFT(1024, FFTW_R2HC),
      mOrder(order),
      mLastSpec(mFFT.getLength() / 2 + 1, 0.0f)
{
}

void LinPred::setOrder(int order)
{
    mOrder = order;
}

inline double G(double x, int L, double alpha)
{
    const int N = L - 1;
    const double k = (x - N / 2.0f) / (2 * L * alpha);
    return expf(-(k * k));
}

static void calcGaussian(std::vector<double>& win, double alpha)
{
    const int L = win.size();
    
    double Gmh = G(-0.5f, L, alpha);
    double GmhpLpGmhmL = G(-0.5f + L, L, alpha) - G(-0.5f - L, L, alpha);

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

    static std::vector<double> window;
    if (window.size() != inLength) {
        constexpr double alpha = 0.2;
        window.resize(inLength);
        calcGaussian(window, alpha);
    }

    auto inData = std::make_unique<double[]>(inLength);

    for (int i = 0; i < inLength; ++i) {
        inData[i] = in->getConstData()[i] * window[i];
    }

    double gain;
    std::vector<double> lpc = mSolver->solve(
            inData.get(),
            inLength,
            mOrder,
            &gain);

    
    out->setSampleRate(sampleRate);

    out->setFFOrder(1);
    out->getFFData()[0] = gain;
    
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

    double max = 1e-10;

    for (int i = 0; i < outLength; ++i) {
        double spec = out->getFFConstData()[0]
                                / (mFFT.data(i) * mFFT.data(i)
                                        + mFFT.data(nfft - 1 - i) * mFFT.data(nfft - 1 - i));
        if (spec > max) {
            max = spec;
        }
        outSpec->getData()[i] = spec;
    }
    
    for (int i = 0; i < outLength; ++i) {
        outSpec->getData()[i] = mLastSpec[i] = 0.2f * mLastSpec[i]
                                                + 0.8f * outSpec->getData()[i] / max;
    }
}
