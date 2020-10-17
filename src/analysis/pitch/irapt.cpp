#include "../util/util.h"
#include "../filter/filter.h"
#include "../resampler/resampler.h"
#include "../../modules/math/constants.h"
#include "irapt/irapt.h"
#include "pitch.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <set>

using namespace Analysis;

PitchResult Pitch::IRAPT::solve(const double *data, int length, int sampleRate)
{
    irapt1();
}

