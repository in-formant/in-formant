#include "spectrum.h"

using namespace Nodes;

Spectrum::Spectrum(int nfft)
    : Node(NodeDescriptor {
            .inputCount = 1,
            .inputs = &kNodeIoTypeAudioTime,
            .outputCount = 1,
            .outputs = &kNodeIoTypeAudioSpec,
        }) ,
      mFFT(new Analysis::RealFFT(nfft))
{
}

void Spectrum::setFFTLength(int nfft)
{
    delete mFFT;
    mFFT = new Analysis::RealFFT(nfft);
}

int Spectrum::getFFTLength() const
{
    return mFFT->getInputLength();
}

void Spectrum::process(const NodeIO *inputs, NodeIO *outputs)
{
    auto in = inputs[0].as<IO::AudioTime>();
    auto out = outputs[0].as<IO::AudioSpec>();

    int inLength = in->getLength();
    int nfft = mFFT->getInputLength();
    int outLength = mFFT->getOutputLength();

    for (int i = 0; i < nfft; ++i) {
        float sample = (i < inLength) ? in->getConstData()[i] : 0.0f;
        float window = 0.5f * (1.0f - cos((2.0f * M_PI * i) / (nfft - 1.0f)));

        mFFT->input(i) = sample * window;
    }

    mFFT->compute();

    out->setSampleRate(in->getSampleRate());
    out->setLength(outLength);

    for (int i = 0; i < outLength; ++i) {
        out->getData()[i] = abs(mFFT->output(i));
    }

}
