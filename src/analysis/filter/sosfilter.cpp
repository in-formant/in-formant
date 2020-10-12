#include "filter.h"

std::vector<float> Analysis::sosfilter(const std::vector<std::array<float, 6>>& sos, const std::vector<float>& x)
{
    std::vector<float> y = x;

    for (const auto& sec : sos) {
        std::vector<float> b {sec[0], sec[1], sec[2]};
        std::vector<float> a {sec[3], sec[4], sec[5]};

        y = filter(b, a, y);
    }

    return y;
}
