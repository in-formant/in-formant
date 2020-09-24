#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include "../nodes/node/nodeio_frequencies.h"
#include "../analysis/filter/filter.h"
#include <vector>
#include <deque>

namespace Synthesis
{
    std::vector<float> whiteNoise(int length);
    std::vector<float> brownNoise(int length, float lastSample);

    std::pair<std::vector<float>, std::vector<float>>
                       generateVTFilter(const Nodes::IO::Frequencies *frequencies,
                                        const Nodes::IO::Frequencies *bandwidths,
                                        float Fs);

    std::vector<float> filter(
                    float b,
                    const std::vector<float>& a,
                    const std::vector<float>& x,
                    std::deque<float>& memoryOut);
}

#endif // SYNTHESIS_H
