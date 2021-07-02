#include "formant.h"
#include "../util/util.h"
#include "../util/laguerre.h"
#include "../fft/fft.h"
#include <algorithm>

using namespace Analysis::Formant;
using Analysis::FormantResult;

static int cauchyIntegral(const rpm::vector<double>& p, double r1, double r2, double phi, int maxDepth);

static rpm::vector<int> peak_picking(const rpm::vector<double> &nsdf);

FormantResult FilteredLP::solve(const double *lpc, int lpcOrder, double sampleRate)
{
    rpm::vector<double> polynomial(lpcOrder + 1);
    polynomial[0] = 1.0;
    std::copy(lpc, lpc + lpcOrder, std::next(polynomial.begin()));

    rpm::vector<std::complex<double>> roots = findRoots(polynomial);
    
    struct FormantRoot {
        FormantData d;
        std::complex<double> r;
    };

    rpm::vector<FormantRoot> pickedRoots;

    const double phiDelta = 2.0 * 50.0 * M_PI / sampleRate;

    for (const auto& z : roots) {
        if (z.imag() < 0) continue;

        double r = std::abs(z);
        double phi = std::arg(z);

        if (r >= 0.6 && r < 1.0) {
            FormantData formant = calculateFormant(r, phi, sampleRate);
            if (formant.frequency > 50.0 && formant.frequency < sampleRate / 2 - 50.0) {
                pickedRoots.push_back({formant, z});
            }
        }
    }

    std::sort(pickedRoots.begin(), pickedRoots.end(),
            [](const auto& a, const auto& b) { return a.d.frequency < b.d.frequency; });

    FormantResult result;
    rpm::vector<FormantRoot> mergedPeaks;

    for (int i = 0; i < int(pickedRoots.size()) - 1; ++i) {
        if (pickedRoots[i + 1].d.frequency - pickedRoots[i].d.frequency > 700
                && (i > 0 || pickedRoots[i + 1].d.frequency > 1800)) {
            mergedPeaks.push_back(pickedRoots[i]);
        }
        else {
            result.formants.push_back(pickedRoots[i].d);
        }
    }
    if (!pickedRoots.empty()) {
        result.formants.push_back(pickedRoots.back().d);
    }

    rpm::vector<std::complex<double>> resolvedRoots;

    for (const auto& v : mergedPeaks) {
        double phiPeak = std::arg(v.r);
        double deltaPhi = 700.0 * 2.0 * M_PI / sampleRate;

        double phi3 = phiPeak - deltaPhi / 2.0;
        double phi4 = phiPeak + deltaPhi / 2.0;

        double minPhi = 200.0 * 2.0 * M_PI / sampleRate;
        if (phi3 < minPhi) {
            phi3 = minPhi;
        }

        int n3 = cauchyIntegral(polynomial, 0, 2, phi3, 100);
        int n4 = cauchyIntegral(polynomial, 0, 2, phi4, 100);

        int n = std::abs(n4 - n3);

        if (n == 2) {
            rpm::vector<std::complex<double>> pr(polynomial.rbegin(), polynomial.rend());
            std::complex<double> r1, r2;
            r1 = Analysis::laguerreRoot(pr, std::polar(0.9, phiPeak), 1e-12);
            pr = Analysis::laguerreDeflate(pr, r1);
            r2 = Analysis::laguerreRoot(pr, std::polar(0.9, phiPeak), 1e-12);

            resolvedRoots.push_back(r1);
            resolvedRoots.push_back(r2);
        }
        else {
            resolvedRoots.push_back(v.r);
        }
    }

    for (const auto& z : resolvedRoots) {
        if (z.imag() < 0) continue;

        double r = std::abs(z);
        double phi = std::arg(z);

        if (r >= 0.6 && r < 1.0) {
            FormantData formant = calculateFormant(r, phi, sampleRate);
            if (formant.frequency > 50.0 && formant.frequency < sampleRate / 2 - 50.0) {
                result.formants.push_back(std::move(formant));
            }
        }
    }

    sortFormants(result.formants);

    return result;
}

static void snellCalcRegion(double t, rpm::map<double, int>& C, const rpm::vector<double>& p, double phi)
{
    // Do not calculate again.
    if (C.find(t) == C.end()) {
        std::complex<double> x = std::polar(t, phi);
        std::complex<double> y, dy;
        Analysis::evaluatePolynomialWithDerivative(p, x, &y, &dy);

        // Find the octant it is in.
        int Ci = static_cast<int>(16 + floor(std::arg(y) / (M_PI / 8.0))) % 8;
        C[t] = Ci;
    }
}

static void snellCalcPartition(const rpm::vector<double>& t, rpm::map<double, int>& C, const rpm::vector<double>& p, double phi, int maxDepth, rpm::vector<double>& partition)
{
    rpm::vector<std::pair<double, double>> partNext;
    rpm::vector<std::pair<double, double>> partCurrent;
    for (int i = 0; i < signed(t.size()) - 1; ++i) {
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
        (void) t1;

        partition.push_back(t2);
    }
}

int cauchyIntegral(const rpm::vector<double>& p, double r1, double r2, double phi, int maxDepth)
{
    // Let C(t) denote the region containing the point P(te^(j*phi)).
    // If ti, i = 0 to i = M is a partition of the ray (r1 -> r2),
    // then p(ti, ti+1) denotes the number of octants (mod 8) between C(ti) and C(ti-1)

    rpm::map<double, int> C; // Memoised counts for every value of t encountered.

    rpm::vector<double> initialPartition({0, 0.2, 0.4, 0.6, 0.8, 1.0, 2.0});
    rpm::vector<double> finalPartition;
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

    for (int i = 0; i < signed(finalPartition.size()) - 1; ++i) {
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

rpm::vector<int> peak_picking(const rpm::vector<double> &nsdf)
{
	rpm::vector<int> max_positions{};
	int pos = 0;
	int cur_max_pos = 0;
	int size = (int) nsdf.size();

	while (pos < (size - 1) / 3 && nsdf[pos] > 0)
		pos++;
	while (pos < size - 1 && nsdf[pos] <= 0.0)
		pos++;

	if (pos == 0)
		pos = 1;

	while (pos < size - 1) {
		if (nsdf[pos] > nsdf[pos - 1] && nsdf[pos] >= nsdf[pos + 1] &&
		    (cur_max_pos == 0 || nsdf[pos] > nsdf[cur_max_pos])) {
			cur_max_pos = pos;
		}
		pos++;
		if (pos < size - 1 && nsdf[pos] <= 0) {
			if (cur_max_pos > 0) {
				max_positions.push_back(cur_max_pos);
				cur_max_pos = 0;
			}
			while (pos < size - 1 && nsdf[pos] <= 0.0) {
				pos++;
			}
		}
	}
	if (cur_max_pos > 0) {
		max_positions.push_back(cur_max_pos);
	}
	return max_positions;
}
