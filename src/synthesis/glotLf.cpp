#include "../modules/math/constants.h"
#include "synthesis.h"
#include <cmath>
#include <map>

template<typename Func, typename Func2>
static double fzero(Func f, Func2 df, double x0);

void Synthesis::lfRd2tetpta(LF_State& state)
{
    const double Rd = state.Rd;

    const double Rap = (-1.0 + 4.8 * Rd) / 100.0;
    const double Rkp = (22.4 + 11.8 * Rd) / 100.0;
    const double Rgp = 1.0 / (4.0 * ((0.11 * Rd / (0.5 + 1.2 * Rkp)) - Rap) / Rkp);

    double tp = 1.0 / (2.0 * Rgp);
    double te = tp * (Rkp + 1.0);
    double ta = Rap;

    state.tp = tp;
    state.te = te;
    state.ta = ta;
}

void Synthesis::lfEpsAlpha(LF_State& state)
{
    const double T0 = state.T0;
    const double Te = state.te * T0;
    const double Tp = state.tp * T0;
    const double Ta = state.ta * T0;
    const double wg = M_PI / Tp;

    // e is expressed by an implicit equation
    const auto fb = [&](double e) {
        return 1.0 - expf(-e * (T0 - Te)) - e * Ta;
    };
    const auto dfb = [&](double e) {
        return (T0 - Te) * expf(-e * (T0 - Te)) - Ta;
    };
    const double e = fzero(fb, dfb, 1 / (Ta + 1e-13));

    // a is expressed by another implicit equation
    // integral{0, T0} ULF(t) dt, where ULF(t) is the LF model equation
    const double A = (1.0 - expf(-e * (T0 - Te))) / (e * e * Ta) - (T0 - Te) * expf(-e * (T0 - Te)) / (e * Ta);
    const auto fa = [&](double a) {
        return (a * a + wg * wg) * sinf(wg * Te) * A + wg * expf(-a * Te) + a * sinf(wg * Te) - wg * cosf(wg * Te);
    };
    const auto dfa = [&](double a) {
        return (2 * A * a + 1) * sinf(wg * Te) - wg * Te * expf(-a * Te);
    };
    const double a = fzero(fa, dfa, 4.42);

    state.eps = e;
    state.alpha = a;
}

std::vector<float> Synthesis::lfGenFrame(float f0, float Fs, float Rd)
{
    const double Ee = 1.0f;
    
    LF_State state;
    state.Rd = Rd;
    state.T0 = 1.0 / f0;

    lfRd2tetpta(state);
    lfEpsAlpha(state);

    const int period = std::round(Fs / f0);

    const double T0 = state.T0;
    const double Te = state.te * T0;
    const double Tp = state.tp * T0;
    const double Ta = state.ta * T0;
    const double a = state.alpha;
    const double e = state.eps;

    std::vector<float> glot(period);
   
    float posMax = 1e-10f;

    for (int i = 0; i < period; ++i) {
        double t = (i * T0) / period;

        if (t <= Te) {
            glot[i] = (-Ee * expf(a * (t - Te)) * sinf(M_PI * t / Tp)) / sinf(M_PI * Te / Tp);
        }
        else {
            glot[i] = -Ee / (e * Ta) * (expf(-e * (t - Te)) - expf(-e * (T0 - Te)));
        }

        if (glot[i] > posMax) {
            posMax = glot[i];
        }
    }

    for (int i = 0; i < period; ++i) {
        glot[i] /= posMax;
    }

    return glot;
}

template<typename Func, typename Func2>
double fzero(Func f, Func2 df, double x0)
{
    constexpr double tol = 1e-7;
    constexpr double eps = 1e-13;
    constexpr int maxIter = 50;

    for (int iter = 0; iter < maxIter; ++iter) 
    {
        double y = f(x0);
        double dy = df(x0);

        if (fabs(dy) < eps) {
            return x0;
        }

        double x1 = x0 - y / dy;

        if (fabs(x1 - x0) <= tol) {
            return x1;
        }

        x0 = x1;
    }

    return x0;
}
