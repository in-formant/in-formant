#ifndef NODES_SPECTRUM_H
#define NODES_SPECTRUM_H

#include "../node/node.h"
#include "../../analysis/fft/fft.h"

namespace Nodes
{
    class Spectrum : public Node {
    public:
        Spectrum(int nfft);

        void setFFTLength(int nfft);
        int getFFTLength() const;

        void process(const NodeIO *inputs[], NodeIO *outputs[]) override;

    private:
        Analysis::ReReFFT *mFFT;
    };
}

#endif // NODES_SPECTRUM_H
