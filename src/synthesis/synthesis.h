#ifndef SYNTHESIS_H
#define SYNTHESIS_H

#include "../nodes/node/nodeio_frequencies.h"
#include "../analysis/filter/filter.h"
#include "../analysis/formant/formant.h"
#include <vector>
#include <deque>

namespace Synthesis
{
    std::vector<float> whiteNoise(int length);
    std::vector<float> brownNoise(int length, float lastSample);

    std::pair<std::vector<float>, std::vector<float>>
                       generateVTFilter(const std::vector<Analysis::FormantData>& formants,
                                        float Fs);

    std::vector<float> filter(
                    const std::vector<float>& b,
                    const std::vector<float>& a,
                    const std::vector<float>& x,
                    std::deque<float>& memoryOut);
}

#endif // SYNTHESIS_H
