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

void Spectrum::process(const NodeIO *inputs[], NodeIO *outputs[])
{
    auto in = inputs[0]->as<IO::AudioTime>();
    auto out = outputs[0]->as<IO::AudioSpec>();

    int inLength = in->getLength();
    int nfft = mFFT->getInputLength();
    int outLength = mFFT->getOutputLength();

    constexpr float a0 = 0.21557895;
    constexpr float a1 = 0.41663158;
    constexpr float a2 = 0.277263158;
    constexpr float a3 = 0.083578947;
    constexpr float a4 = 0.006947368;

    const int N = nfft - 1;

    for (int i = 0; i < nfft; ++i) {
        float sample = (i < inLength) ? in->getConstData()[i] : 0.0f;
        float window = a0
                        - a1 * cos((2.0f * M_PI * i) / N)
                        + a2 * cos((4.0f * M_PI * i) / N)
                        - a3 * cos((6.0f * M_PI * i) / N)
                        + a4 * cos((8.0f * M_PI * i) / N);

        mFFT->input(i) = sample * window;
    }

    mFFT->compute();

    out->setSampleRate(in->getSampleRate());
    out->setLength(outLength);

    for (int i = 0; i < outLength; ++i) {
        out->getData()[i] = abs(mFFT->output(i));
    }

}
