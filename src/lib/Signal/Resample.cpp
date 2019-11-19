//
// Created by rika on 11/10/2019.
//

#include "Resample.h"
#include "../FFT/FFT.h"

using namespace Eigen;

ArrayXd Resample::resample(const ArrayXd & x, double sourceFs, double targetFs, int precision)
{
    double upfactor = targetFs / sourceFs;

    if (std::abs(upfactor - 2.0) < 1e-6) return Resample::upsample(x);
    if (std::abs(upfactor - 1.0) < 1e-6) return x;

    int numberOfSamples = std::round((x.size() * targetFs) / sourceFs);
    if (numberOfSamples < 1) {
        return ArrayXd::Zero(0);
    }

    const int nx = x.size();
    ArrayXd z;
    if (upfactor < 1.0) {
        // Anti-aliasing filter.
        constexpr int antiTurnAround = 1000;
        constexpr int numberOfPaddingSides = 2;
        int nfft = 1;
        while (nfft < nx + antiTurnAround * numberOfPaddingSides) nfft *= 2;

        rfft_plan(nfft);
        Map<ArrayXd> data(rfft_in(nfft), nfft);
        Map<ArrayXd> data2(rfft_out(nfft), nfft);
        data.setZero();
        data.segment(antiTurnAround, nx) = x;
        rfft(nfft); // go to frequency domain
        for (int i = std::floor(upfactor * nfft); i < nfft; ++i)
            data2(i) = 0.0; // filter away high frequencies
        data2(0) = 0.0;
        irfft_plan(nfft);
        Map<ArrayXd>(irfft_in(nfft), nfft) = data2;
        Map<ArrayXd> data3(irfft_out(nfft), nfft);
        irfft(nfft); // return to time domain

        z = data3.segment(antiTurnAround, nx) / static_cast<double>(2 * nfft);
    }
    else {
        z = x;
    }

    ArrayXd y(numberOfSamples);

    if (precision <= 1) {
        for (int i = 0; i < numberOfSamples; ++i) {
            double index = (i * sourceFs) / targetFs;
            int leftSample = std::floor(index);
            double fraction = index - leftSample;
            y(i) = (leftSample < 0 || leftSample >= nx - 1) ? 0.0 :
                    (1 - fraction) * z(leftSample) + fraction * z(leftSample + 1);
        }
    }
    else {
        for (int i = 0; i < numberOfSamples; ++i) {
            double index = (i * sourceFs) / targetFs;
            y(i) = Resample::interpolate_sinc(z, index, precision);
        }
    }

    return y;

}

ArrayXd Resample::upsample(const ArrayXd & x)
{
    const int nx = x.size();

    constexpr int antiTurnAround = 1000;
    constexpr int sampleRateFactor = 2;
    constexpr int numberOfPaddingSides = 2;
    int nfft = 1;
    while (nfft < nx + antiTurnAround * numberOfPaddingSides) nfft *= 2;

    const int numberOfSamples = nx * sampleRateFactor;

    int n = sampleRateFactor * nfft;

    rfft_plan(n);
    Map<ArrayXd> data(rfft_in(n), n);
    Map<ArrayXd> data2(rfft_out(n), n);
    data.setZero();
    data.segment(antiTurnAround, nx) = x;
    rfft(n); // go to frequency domain
    int imin = std::floor(nfft * 0.95);
    for (int i = imin; i < nfft; ++i)
        data[i] *= static_cast<double>(nfft - i) / static_cast<double>(nfft - imin);
    data[0] = 0.0;
    irfft_plan(n);
    Map<ArrayXd>(irfft_in(n), n) = data2;
    Map<ArrayXd> data3(irfft_out(n), n);
    irfft(n); // return to time domain
    ArrayXd z = data3.segment(antiTurnAround, nx) / static_cast<double>(nfft);

    return z;
}

double Resample::interpolate_sinc(const ArrayXd & y, double x, int maxDepth)
{
    int ix, midleft = std::floor(x), midright = midleft + 1, left, right;
    double result = 0.0, a, halfsina, aa, daa;

    // simple cases
    if (y.size() < 1) return NAN;
    if (x >= y.size()) return y(y.size() - 1);
    if (x < 0) return y(0);
    if (x == midleft) return y(midleft);
    // 1 < x < y.size && x not integer: interpolate
    if (maxDepth > midright - 1) maxDepth = midright - 1;
    if (maxDepth >= y.size() - midleft) maxDepth = y.size() - midleft - 1;
    if (maxDepth <= Resample::Nearest) return y(static_cast<int>(std::floor(x + 0.5)));
    if (maxDepth == Resample::Linear) return y(midleft) + (x - midleft) * (y(midright) - y(midleft));
    if (maxDepth == Resample::Cubic) {
        double yl = y(midleft), yr = y(midright);
        double dyl = 0.5 * (yr - y(midleft - 1)), dyr = 0.5 * (y(midright+ 1) - yl);
        double fil = x - midleft, fir = midright - x;
        return yl * fir + yr * fil - fil * fir * (0.5 * (dyr - dyl) + (fil - 0.5) * (dyl + dyr - 2 * (yr - yl)));
    }

    left = midright - maxDepth;
    right = midleft + maxDepth;
    a = M_PI * (x - midleft);
    halfsina = 0.5 * std::sin(a);
    aa = a / (x - left + 1);
    daa = M_PI / (x - left + 1);
    for (ix = midleft; ix >= left; --ix) {
        double d = halfsina / a * (1.0 + std::cos(aa));
        result += y(ix) * d;
        a += M_PI;
        aa += daa;
        halfsina = -halfsina;
    }

    a = M_PI * (midright - x);
    halfsina = 0.5 * std::sin(a);
    aa = a / (right - x + 1);
    daa = M_PI / (right - x + 1);
    for (ix = midright; ix <= right; ++ix) {
        double d = halfsina / a * (1.0 + std::cos(aa));
        result += y(ix) * d;
        a += M_PI;
        aa += daa;
        halfsina = -halfsina;
    }

    return result;
}
