#ifndef ANALYSIS_RESAMPLER_H
#define ANALYSIS_RESAMPLER_H

#include <vector>

namespace Analysis
{
    std::vector<double> resample(const std::vector<double>& x, double srcFs, double dstFs);
}

#endif // ANALYSIS_RESAMPLER_H
