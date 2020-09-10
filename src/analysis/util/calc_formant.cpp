#include "util.h"
#include "../../modules/math/constants.h"

Analysis::FormantData Analysis::calculateFormant(float r, float phi, float sampleRate)
{
    return {
        .frequency = (float) (fabs(phi) * sampleRate / (2.0f * M_PI)),
        .bandwidth = (float) (-log(r) * sampleRate / M_PI), 
    };
}
