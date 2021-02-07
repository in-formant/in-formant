#include "synthesis.h"

rpm::vector<double> Synthesis::sosfilter(const rpm::vector<std::array<double, 6>>& sos, const rpm::vector<double>& x, rpm::vector<rpm::vector<double>>& zf)
{
    rpm::vector<double> y = x;

    int i = 0;
    for (const auto& sec : sos) {
        rpm::vector<double> b {sec[0], sec[1], sec[2]};
        rpm::vector<double> a {sec[3], sec[4], sec[5]};

        y = filter(b, a, y, zf[i]);
        i++;
    }

    return y;
}
