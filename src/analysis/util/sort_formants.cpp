#include "util.h"
#include <algorithm>

void Analysis::sortFormants(rpm::vector<FormantData>& formants)
{
    std::sort(formants.begin(), formants.end(),
            [](const auto& x, const auto& y) { return x.frequency < y.frequency; });
}
