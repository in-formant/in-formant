#include "freqz.h"
#include "../util/util.h"

rpm::vector<double> Analysis::sosfreqz(const rpm::vector<std::array<double, 6>>& sos, const rpm::vector<std::complex<double>>& wn)
{
    rpm::vector<double> h(wn.size(), 1.0);

    for (const auto& sec : sos) {
        const double a0 = sec[3];

        rpm::vector<double> b {sec[2] / a0, sec[1] / a0, sec[0] / a0};
        rpm::vector<double> a {sec[5] / a0, sec[4] / a0, sec[3]};

        rpm::vector<std::complex<double>> secNum = evaluatePolynomialComplexVector(b, wn);
        rpm::vector<std::complex<double>> secDen = evaluatePolynomialComplexVector(a, wn);

        for (int k = 0; k < wn.size(); ++k) {
            h[k] *= std::abs(secNum[k] / secDen[k]);
        }
    }

    return h;
}
