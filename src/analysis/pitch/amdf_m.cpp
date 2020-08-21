#include "pitch.h"
#include "../util/util.h"
#include <cmath>
#include <limits>

using Analysis::PitchResult;
using namespace Analysis::Pitch;

AMDF_M::AMDF_M(float minPitch, float maxPitch, float alpha)
    : mMinPitch(minPitch),
      mMaxPitch(maxPitch),
      mAlpha(alpha)
{
}

PitchResult AMDF_M::solve(const float *data, int length, int sampleRate)
{
    const int maxShift = length;

    const int maxPeriod = std::min<int>(ceil(sampleRate / mMinPitch), maxShift - 1);
    const int minPeriod = std::max<int>(floor(sampleRate / mMaxPitch), 2);
    
    // Calculate the AMDF.
    mAMDF.resize(maxShift);
    
    float Vmax = 0.0f;
    float Vmin = std::numeric_limits<float>::max();

    for (int i = 0; i < maxShift; ++i) {
        float sum = 0.0f;

        for (int j = 0; j < maxShift - i; ++j) {
            sum += fabs(data[j] - data[i + j]);
        }

        mAMDF[i] = sum / (maxShift - i);

        if (mAMDF[i] > Vmax) {
            Vmax = mAMDF[i];
        }
        if (mAMDF[i] < Vmin) {
            Vmin = mAMDF[i];
        }
    }

    // Convert to 1-bit AMDF.
    const float theta = mAlpha * (Vmax + Vmin);

    m1bAMDF.resize(maxShift / 32 + 1);

    for (int k = 0; k < maxShift / 32 + 1; ++k) {
        uint32_t v = 0u;

        for (int bit = 0; bit < 32; ++bit) {
            int i = k * 32 + bit;
            if (i < maxShift && mAMDF[i] <= theta) {
                v |= 1u << bit;
            }
        }

        m1bAMDF[k] = v;
    }

    // Calculate the ACF for the 1-bit AMDF signal.
    m1bACF.resize(maxShift);

    for (int i = 0; i < maxShift; ++i) {
        int sum = 0;

        for (int j = 0; j < maxShift - i; ++j) {
            int aInd = j / 32;
            int aBit = j % 32;

            int bInd = (i + j) / 32;
            int bBit = (i + j) % 32;

            if ((m1bAMDF[aInd] & (1u << aBit)) && (m1bAMDF[bInd] & (1u << bBit)))
                sum++;
        }

        m1bACF[i] = sum;
    }

    // Find the global peak.
    std::vector<int> maxPositions = findPeaks(
            std::next(m1bACF.data(), minPeriod),
            maxPeriod - minPeriod + 1);

    if (maxPositions.empty()) {
        return {.pitch = 0.0f, .voiced = false};
    }

    const float actualCutoff = mAlpha * m1bACF[0];

    float pitch = 0.0f;

    for (const int pos : maxPositions) {
        int i = minPeriod + pos;

        if (m1bACF[i] >= actualCutoff) {
            pitch = (float) sampleRate / (float) i;

            if (pitch >= mMinPitch && pitch <= mMaxPitch)
                return {.pitch = pitch, .voiced = true};
        }
    }

    return pitch > 0.0f ? PitchResult {.pitch = pitch, .voiced = true}
                        : PitchResult {.pitch = 0.0f, .voiced = false};
}
