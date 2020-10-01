#ifndef ANALYSIS_RESAMPLER_H
#define ANALYSIS_RESAMPLER_H

#include <vector>

namespace Analysis
{
    std::vector<float> resample(const std::vector<float>& x, float srcFs, float dstFs);
}

#endif // ANALYSIS_RESAMPLER_H
