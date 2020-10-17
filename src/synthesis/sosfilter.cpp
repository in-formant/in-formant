#include "synthesis.h"

std::vector<double> Synthesis::sosfilter(const std::vector<std::array<double, 6>>& sos, const std::vector<double>& x, std::vector<std::vector<double>>& zf)
{
    std::vector<double> y = x;

    int i = 0;
    for (const auto& sec : sos) {
        std::vector<double> b {sec[0], sec[1], sec[2]};
        std::vector<double> a {sec[3], sec[4], sec[5]};

        y = filter(b, a, y, zf[i]);
        i++;
    }

    return y;
}
