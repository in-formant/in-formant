#include "synthesis.h"

std::vector<double> Synthesis::filter(
        const std::vector<double>& b,
        const std::vector<double>& a,
        const std::vector<double>& x,
        std::vector<double>& zf)
{
    const int la = a.size();
    const int lb = b.size();
    const int lab = std::max(la, lb);

    std::vector<double> bp = b;
    std::vector<double> ap = a;
    bp.resize(lab, 0.0f);
    ap.resize(lab, 0.0f);

    for (int i = 0; i < lab; ++i) {
        bp[i] /= a[0];
        ap[i] /= a[0];
    }

    const int lz = lab - 1;
    const int lx = x.size();

    std::vector<double> y(lx);
    zf.resize(lz);

    if (la > 1) {
        for (int i = 0; i < lx; ++i) {
            y[i] = zf[0] + bp[0] * x[i];

            if (lz > 0) {
                for (int j = 0; j < lz - 1; ++j)
                    zf[j] = zf[j + 1] + bp[j + 1] * x[i] - ap[j + 1] * y[i];
                zf[lz - 1] = bp[lz] * x[i] - ap[lz] * y[i];
            }
            else {
                zf[0] = bp[lz] * x[i] - ap[lz] * y[i];
            }
        }
    }
    else if (lz > 0) {
        for (int i = 0; i < lx; ++i) {
            y[i] = zf[0] + bp[0] * x[i];

            if (lz > 1) {
                for (int j = 0; j < lz - 1; ++j)
                    zf[j] = zf[j + 1] + bp[j + 1] * x[i];
                zf[lz - 1] = bp[lz] * x[i];
            }
            else {
                zf[0] = bp[1] * x[i];
            }
        }
    }

    return std::vector<double>(y.begin(), y.end());
}

