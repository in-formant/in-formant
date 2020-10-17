#include "filter.h"

std::vector<double> Analysis::sosfilter(const std::vector<std::array<double, 6>>& sos, const std::vector<double>& x)
{
    std::vector<double> y = x;

    for (const auto& sec : sos) {
        std::vector<double> b {sec[0], sec[1], sec[2]};
        std::vector<double> a {sec[3], sec[4], sec[5]};

        y = filter(b, a, y);
    }

    return y;
}
