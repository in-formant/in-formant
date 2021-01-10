#include "sigma.h"
#include "../filter/filter.h"
#include "../util/util.h"
#include "../../modules/math/constants.h"
#include <limits>
#include <algorithm>

constexpr double eps = std::numeric_limits<double>::epsilon();

std::tuple<std::vector<double>, std::vector<double>, std::vector<double>, int>
SIGMA::xewgrdel(const std::vector<double>& u, double fs, double gwlen, double fwlen)
{
    // Force window length to be odd.
    int gw = 2 * (gwlen * fs / 2) + 1;

    std::vector<double> ghw(gw);
    std::vector<double> ghwn(gw);

    for (int i = 0, j = gw - 1; i < gw; ++i, j -= 2) {
        ghw[i] = 0.54 + 0.46 * cos(2 * M_PI / (gw - 1) * i);
        ghwn[i] = ghw[i] * j / 2.0;
    }

    std::vector<double> u2(u.size());
    for (int i = 0; i < u.size(); ++i) {
        u2[i] = u[i] * u[i];
    }

    std::vector<double> yn = Analysis::filter(ghwn, u2);
    std::vector<double> yd = Analysis::filter(ghw, u2);

    // Prevent infinities.
    std::replace_if(yd.begin(), yd.end(),
            [](double x) { return fabs(x) < eps; },
            10 * eps);
    
    std::vector<double> y(u.size() - gw);
    for (int i = gw; i < u.size(); ++i) {
        y[i - gw] = yn[i] / yd[i];
    }
    
    int toff = (gw - 1) / 2;

    // Force window length to be odd
    int fw = 2 * (fwlen * fs / 2) + 1;

    if (fw > 1) {
        std::vector<double> daw(fw);
        double sum = 0.0;

        for (int i = 0; i < fw; ++i) {
            daw[i] = 0.54 + 0.46 * cos(2 * M_PI / (fw - 1) * i);
            sum += daw[i];
        }

        // Low pass filter
        y = Analysis::filter(daw, y);
        for (int i = 0; i < y.size(); ++i) {
            y[i] /= sum;
        }

        toff -= (fw - 1) / 2;
    }

    auto [tew, sew] = Analysis::findZerocros(y, 'n');

    for (auto& t : tew) {
        t += toff;
    }

    // [tew, sew, y, toff]
    return std::make_tuple(tew, sew, y, toff);
}
