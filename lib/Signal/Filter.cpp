//
// Created by clo on 12/09/2019.
//

#include "Filter.h"
#include "../Math/Polynomial.h"

using namespace Eigen;

void Filter::responseFIR(const ArrayXd & a, const ArrayXd & f, double fs, ArrayXcd & h) {

    ArrayXcd z = exp((std::complex<double>(0, -1) * 2.0 * M_PI * f) / fs);

    Polynomial::evaluate(a, z, h);

}

void Filter::responseIIR(const ArrayXd & b, const ArrayXd & a, const ArrayXd & f, double fs, ArrayXcd & h) {

    ArrayXcd z = exp((std::complex<double>(0, -1) * 2.0 * M_PI * f) / fs);

    // Postpad the coefficients with zeroes.

    int lenb = b.size();
    int lena = a.size();

    ArrayXd b_pad = b;
    ArrayXd a_pad = a;

    if (lena < lenb) {
        a_pad.conservativeResize(lenb);
        a_pad.tail(lenb - lena).setZero();
    }
    else {
        b_pad.conservativeResize(lena);
        b_pad.tail(lena - lenb).setZero();
    }

    ArrayXcd hb, ha;

    Polynomial::evaluate(b, z, hb);
    Polynomial::evaluate(a, z, ha);

    h = hb / ha;

}

void Filter::preEmphasis(ArrayXd & x, double samplingFrequency, double preEmphasisFrequency)
{
    if (preEmphasisFrequency >= 0.5 * samplingFrequency)
        return;

    const double preEmphasis = 1.0 / (2.0 * M_PI * preEmphasisFrequency / samplingFrequency + 1.0);

    ArrayXd y(x.size());

    y(0) = x(0);
    for (int i = 1; i < x.size(); ++i) {
        y(i) = preEmphasis * (y(i - 1) + x(i) - x(i - 1));
    }

    x = std::move(y);
}

