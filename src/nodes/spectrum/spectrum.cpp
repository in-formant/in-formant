#include "spectrum.h"
#include "../../modules/math/constants.h"

using namespace Nodes;

constexpr int maxNfft = 16384;

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

Spectrum::~Spectrum()
{
    delete mFFT;
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
    
    if (inLength <= nfft) {
        for (int i = 0; i < nfft; ++i) {
            mFFT->data(i) = 0.0;
        }

        const int N = inLength - 1;

        for (int j = 0; j < inLength; ++j) {
            double sample = in->getConstData()[j];
            double window = 0.5 - 0.5 * cosf((2.0 * M_PI * j) / N);

            int i = nfft / 2 - inLength / 2 + j;

            mFFT->data(i) = sample * window;
        }
    }
    else {
        const int N = nfft - 1;
        
        for (int j = 0; j < nfft; ++j) {
            int i = inLength / 2 - nfft / 2 + j;
            
            double sample = in->getConstData()[i];
            double window = 0.5 - 0.5 * cosf((2.0 * M_PI * j) / N);

            mFFT->data(j) = sample * window;
        }
    }

    mFFT->compute();

    int outLength = nfft / 2 + 1;

    out->setSampleRate(in->getSampleRate());
    out->setLength(outLength);

    static double holdMax = 1.0;

    double max = 1.0;

    for (int i = 0; i < outLength; ++i) {
        double spec = mFFT->data(i) * mFFT->data(i)
                            + mFFT->data(nfft - 1 - i) * mFFT->data(nfft - 1 - i);
        out->getData()[i] = spec;
        if (spec > max) {
            max = spec;
        }
    }
    
    holdMax = max = std::max(0.995 * holdMax + 0.005 * max, max);

    for (int i = 0; i < outLength; ++i) {
        out->getData()[i] /= max;
    }
}
