//
// Created by clo on 08/11/2019.
//

#include <iostream>
#include "Formant.h"
#include "../Math/Polynomial.h"

using namespace Eigen;

struct root { double r, phi; };

static dcomplex intSimps(const ArrayXd & p, double r1, double r2, double phi, int N = 50);

void Formant::sort(Frame & frm)
{
    std::sort(frm.formant.begin(), frm.formant.end(),
               [](const auto & x, const auto & y) {
                    return x.frequency < y.frequency;
                });
}

void Formant::frameFromRoots(
        const Eigen::ArrayXd & p,
        const Eigen::ArrayXcd & r, Frame & frm,
        double samplingFrequency, double margin)
{
    frm.formant.clear();
    frm.formant.reserve(r.size());

    double fLow = margin, fHigh = samplingFrequency / 2.0 - margin;

    std::vector<root> roots;

    std::cout << "=== one frame" << std::endl;

    for (const auto & v : r) {
        if (v.imag() < 0) {
            continue;
        }

        double r = std::abs(v);
        double phi = std::arg(v);

        // Check that the frequency isn't aberrant.
        double f = std::abs(phi) * samplingFrequency / (2.0 * M_PI);
        if (f <= fLow || f >= fHigh) {
            continue;
        }

        // Magnitude condition for forming a formant
        if (0.7 <= r && r < 1.0) {
            roots.push_back({r, phi});
        }
    }

    std::sort(roots.begin(), roots.end(),
            [](const auto & a, const auto & b) { return std::abs(a.phi) < std::abs(b.phi); });

    std::vector<root> peakMergers;

    int ncand = roots.size();
    for (int i = 0; i < ncand - 1; ++i) {
        double phi1 = roots[i].phi;
        double phi2 = roots[i + 1].phi;

        double f1 = std::abs(phi1) * samplingFrequency / (2.0 * M_PI);
        double f2 = std::abs(phi2) * samplingFrequency / (2.0 * M_PI);

        // Phase condition for peak merger
        if (std::abs(phi1 - phi2) > 0.5498) {
            peakMergers.push_back(roots[i]);
        }
        else if (i == 0 && f2 > 1800.0) {
            peakMergers.push_back(roots[i]);
        }
        else {
            // It's a single formant.
            double b1 = -std::log(roots[i].r) * samplingFrequency / M_PI;
            frm.formant.push_back({f1, b1});
        }
    }

    // Integrate for peak mergers.
    for (const auto & r : peakMergers) {
        double phiPeak = r.phi;
        double phi3 = phiPeak - 0.5498 / 2.0;
        double phi4 = phiPeak + 0.5498 / 2.0;

        // Curve 1:
        dcomplex z1 = intSimps(p, 0, 2, phi3);

        // Curve 2:
        dcomplex z2 = (p.size() - 1) * std::abs(phi4 - phi3);

        // Curve 3:
        dcomplex z3 = intSimps(p, 0, 2, phi4);

        // Number of poles in that section.
        double nf = std::real(1.0 / (2.0 * M_PI * dcomplex(0.0, -1.0)) * (z1 + z2 + z3));
        int n = std::trunc(nf);

        std::cout << "n = " << nf << std::endl;

        // If there is only one pole in the section, add it as a single formant.
        if (n == 1) {
            double f = std::abs(phiPeak) * samplingFrequency / (2.0 * M_PI);
            double b = -std::log(r.r) * samplingFrequency / M_PI;
            frm.formant.push_back({f, b});
        }
        else if (n == 2) {
            double f1 = std::abs(phi3) * samplingFrequency / (2.0 * M_PI);
            double f2 = std::abs(phi4) * samplingFrequency / (2.0 * M_PI);
            double b = -std::log(0.9) * samplingFrequency / M_PI;
            frm.formant.push_back({f1, b});
            frm.formant.push_back({f2, b});
        }
        else {
            std::cout << "Warning: no pole or more than two poles detected in a single section" << std::endl;

            double f = std::abs(phiPeak) * samplingFrequency / (2.0 * M_PI);
            double b = -std::log(r.r) * samplingFrequency / M_PI;
            frm.formant.push_back({f, b});
        }
    }

    /*
       double f = std::abs(std::arg(v)) * samplingFrequency / (2.0 * M_PI);
       double b = -std::log(std::abs(v)) * samplingFrequency / M_PI;
     */
    frm.nFormants = frm.formant.size();
    ::Formant::sort(frm);
}

static dcomplex intSimps(const ArrayXd & p, double r1, double r2, double phi, int N)
{
    assert(N % 2 == 0 && "N must be an even integer");

    dcomplex dz = std::polar((r2 - r1) / static_cast<double>(N), phi);

    ArrayXcd z(N + 1);
    for (int i = 0; i <= N; ++i) {
        z(i) = r1 + (double) i * dz;
    }

    ArrayXcd y, dy;
    Polynomial::evaluateWithDerivative(p, z, y, dy);

    ArrayXcd f = dy / y;

    dcomplex left = dz / 2.0 * (f(0) + f(N));
    dcomplex S = left + dz * f(seq(1, N - 1)).sum();

    return S;

}
