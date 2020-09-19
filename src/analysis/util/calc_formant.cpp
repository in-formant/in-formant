#include "util.h"
#include "../../modules/math/constants.h"

Analysis::FormantData Analysis::calculateFormant(float r, float phi, float sampleRate)
{
    return {
        .frequency = (float) (fabsf(phi) * sampleRate / (2.0f * M_PI)),
        .bandwidth = (float) (-logf(r) * sampleRate / M_PI), 
    };
}
