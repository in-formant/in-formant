#include "util.h"
#include "../../modules/math/constants.h"

Analysis::FormantData Analysis::calculateFormant(double r, double phi, double sampleRate)
{
    return {
        .frequency = (double) (fabs(phi) * sampleRate / (2.0 * M_PI)),
        .bandwidth = (double) (-log(r) * sampleRate / M_PI), 
    };
}
