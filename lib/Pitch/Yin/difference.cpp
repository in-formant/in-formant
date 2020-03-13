#include "YIN.h"

using namespace Eigen;

ArrayXd YIN::difference(const ArrayXd & x)
{
    const int N = x.size();

    ArrayXd acorr = autocorrelation(x);

    ArrayXd yin_buffer(N / 2);

    for (int tau = 0; tau < N / 2; ++tau) {
        yin_buffer(tau) = acorr(0) + acorr(1) - 2 * acorr(tau);
    }

    return std::move(yin_buffer);
}
