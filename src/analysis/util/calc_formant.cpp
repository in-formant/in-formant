#include "util.h"

Analysis::FormantData Analysis::calculateFormant(double r, double phi, double sampleRate)
{
    return {
        (double) (fabs(phi) * sampleRate / (2.0 * M_PI)),
        (double) (-log(r) * sampleRate / M_PI), 
    };
}
