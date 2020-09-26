#include "spectrum.h"
#include "../../modules/math/constants.h"

using namespace Nodes;

Spectrum::Spectrum(int nfft)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioSpec,
        }) ,
      mFFT(new Analysis::ReReFFT(nfft, FFTW_R2HC))
{
}

void Spectrum::setFFTLength(int nfft)
{
    delete mFFT;
    mFFT = new Analysis::ReReFFT(nfft, FFTW_R2HC);
}

int Spectrum::getFFTLength() const
{
    return mFFT->getLength();
}

void Spectrum::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioSpec>();

    int inLength = in->getLength();
    int nfft = mFFT->getLength();

    const int N = inLength - 1;

    for (int i = 0; i < nfft; ++i) {
        mFFT->data(i) = 0.0f;
    }

    for (int j = 0; j < inLength; ++j) {
        float sample = in->getConstData()[j];
        float window = 0.5f - 0.5f * cosf((2.0f * M_PI * j) / N);

        int i = nfft / 2 - inLength / 2 + j;

        mFFT->data(i) = sample * window;
    }

    mFFT->compute();

    int outLength = nfft / 2 + 1;

    out->setSampleRate(in->getSampleRate());
    out->setLength(outLength);

    for (int i = 0; i < outLength; ++i) {
        out->getData()[i] = mFFT->data(i) * mFFT->data(i)
                            + mFFT->data(nfft - 1 - i) * mFFT->data(nfft - 1 - i);
    }

}
