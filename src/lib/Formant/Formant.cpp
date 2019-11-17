//
// Created by clo on 08/11/2019.
//

#include <map>
#include <stack>
#include <iostream>
#include <iterator>
#include "Formant.h"
#include "../Math/Polynomial.h"
#include "../Math/Bairstow.h"

using namespace Eigen;

struct root {
    double r, phi;
    double f, b;
};

static int cauchyIntegral(const ArrayXd & p, double r1, double r2, double phi, int maxDepth);

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
        double samplingFrequency)
{
    frm.formant.clear();
    frm.formant.reserve(r.size());

    std::vector<root> roots;
    std::vector<root> peakMergers;
    std::vector<dcomplex> finalRoots;

    for (const auto & v : r) {
        if (v.imag() < 0) {
            continue;
        }

        double r = std::abs(v);
        double phi = std::arg(v);

        // Magnitude condition for forming a formant
        if (0.7 <= r && r < 1.0) {
            double f = std::abs(phi) * samplingFrequency / (2.0 * M_PI);
            double b = -std::log(r) * samplingFrequency / M_PI;
            roots.push_back({r, phi, f, b});
        }
    }

    std::sort(roots.begin(), roots.end(),
            [](const auto & a, const auto & b) { return std::abs(a.phi) < std::abs(b.phi); });

    int ncand = roots.size();
    for (int i = 0; i < ncand - 1; ++i) {
        double f1 = roots[i].f;
        double f2 = roots[i + 1].f;

        // Phase condition for peak merger
        if (std::abs(f2 - f1) > 700.0) {
            peakMergers.push_back(roots[i]);
        }
        else if (i == 0 && f2 > 1800.0) {
            peakMergers.push_back(roots[i]);
        }
        else {
            // It's a single formant.
            frm.formant.push_back({f1, roots[i].b});
        }
    }

    // Calculate how many poles are exactly in peak merger candidate.
    for (const auto & v : peakMergers) {
        double phiPeak = v.phi;
        double deltaPhi = 700 * 2.0 * M_PI / samplingFrequency;

        double phi3 = phiPeak - deltaPhi / 2.0;
        double phi4 = phiPeak + deltaPhi / 2.0;

        double minPhi = 200 * 2.0 * M_PI / samplingFrequency;
        if (phi3 < minPhi) {
            phi3 = minPhi;
        }

        int n3 = cauchyIntegral(p, 0, 2, phi3, 10);
        int n4 = cauchyIntegral(p, 0, 2, phi4, 10);

        int n = abs(n4 - n3);

        // If there *are* two poles in the section, polish them as a pair and add them.
        if (n == 2) {
            std::vector<dcomplex> polished;
            Bairstow::solve(p, 0.9, phiPeak, polished);
            finalRoots.insert(finalRoots.end(), polished.begin(), polished.end());
        }
        else {
            frm.formant.push_back({v.f, v.b});
        }
    }

    for (const auto & v : finalRoots) {
        double r = std::abs(v);

        // Magnitude test
        if (0.7 <= r && r < 1.0) {
            double phi = std::arg(v);

            double f = std::abs(phi) * samplingFrequency / (2.0 * M_PI);
            double b = -std::log(r) * samplingFrequency / M_PI;

            frm.formant.push_back({f, b});
        }
    }

    frm.nFormants = frm.formant.size();
    ::Formant::sort(frm);
}

static void snellCalcRegion(double t, std::map<double, int> & C, const ArrayXd & p, double phi)
{
    // Do not calculate again.
    if (C.find(t) == C.end()) {
        dcomplex x = std::polar(t, phi);
        dcomplex y;
        Polynomial::evaluate(p, x, y);

        // Find the octant it is in.
        int Ci = static_cast<int>(16 + floor(std::arg(y) / (M_PI / 8.0))) % 8;
        C[t] = Ci;
    }
}

static void snellCalcPartition(const std::vector<double> & t, std::map<double, int> & C, const ArrayXd & p, double phi, int maxDepth, std::vector<double> & partition)
{
    std::vector<std::pair<double, double>> partNext;
    std::vector<std::pair<double, double>> partCurrent;
    for (int i = 0; i < t.size() - 1; ++i) {
        partCurrent.emplace_back(t[i], t[i + 1]);
    }

    bool converged = false;
    int depth = 0;

    while (!converged) {
        for (const auto &[t1, t2] : partCurrent) {
            snellCalcRegion(t1, C, p, phi);
            snellCalcRegion(t2, C, p, phi);

            int count = abs(C[t1] - C[t2]) % 8;
            if (count <= 1) {
                partNext.emplace_back(t1, t2);
            } else {
                double tmid = (t1 + t2) / 2.0;
                partNext.emplace_back(t1, tmid);
                partNext.emplace_back(tmid, t2);
            }
        }

        depth++;
        converged = depth > maxDepth || std::equal(partCurrent.begin(), partCurrent.end(), partNext.begin());
        partCurrent = partNext;
        partNext.resize(0);
    }

    partition.push_back(partCurrent.at(0).first);
    for (const auto & [t1, t2] : partCurrent) {
        partition.push_back(t2);
    }
}

static int cauchyIntegral(const ArrayXd & p, double r1, double r2, double phi, int maxDepth)
{
    // Let C(t) denote the region containing the point P(te^(j*phi)).
    // If ti, i = 0 to i = M is a partition of the ray (r1 -> r2),
    // then p(ti, ti+1) denotes the number of octants (mod 8) between C(ti) and C(ti-1)

    std::map<double, int> C; // Memoised counts for every value of t encountered.

    std::vector<double> initialPartition({0, 0.2, 0.4, 0.6, 0.8, 1.0, 2.0});
    std::vector<double> finalPartition;
    snellCalcPartition(initialPartition, C, p, phi, maxDepth, finalPartition);

    /*for (int i = 0; i < finalPartition.size(); ++i) {
        double t = finalPartition[i];
        std::cout << "C(" << t << ") = " << C[t] << std::endl;
    }
    std::cout << std::endl;*/

    // Calculate N+ and N-
    //  N+ is the number of transitions from region C7 to region C0
    //  N- is the number of transitions from region C0 to region C7

    int Np = 0, Nm = 0;

    for (int i = 0; i < finalPartition.size() - 1; ++i) {
        double t1 = finalPartition[i];
        double t2 = finalPartition[i + 1];
        // Make sure all C values are calculated for the final partition.
        snellCalcRegion(t1, C, p, phi);
        snellCalcRegion(t2, C, p, phi);

        int c1 = C[t1];
        int c2 = C[t2];

        if (c1 == 7 && c2 == 0) {
            Np++;
        }
        else if (c1 == 0 && c2 == 7) {
            Nm++;
        }
    }

    int N = Np - Nm;

    return N;
}
