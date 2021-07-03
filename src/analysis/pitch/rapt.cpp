#include "../util/util.h"
#include "../filter/filter.h"
#include "rapt.h"
#include "pitch.h"
#include <samplerate.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

using namespace Analysis;

Pitch::RAPT::RAPT()
{
    F0min   = 50;
    F0max   = 600;
    cand_tr = 0.3;
    lag_wt  = 0.3;
    freq_wt = 0.02;
    vtran_c = 0.005;
    vtr_a_c = 0.5;
    vtr_s_c = 0.5;
    vo_bias = 0.0;
    doubl_c = 0.35;
    a_fact  = 10000;
    n_cands = 20;

    nbFrames = 1;
}

PitchResult Pitch::RAPT::solve(const double *data, int length, int sampleRate)
{
    double f0 = computeFrame(data, length, sampleRate);

    if (f0 != 0.0) {
        return {f0, true};
    }
    else {
        return {0.0, false};
    }
}

static void subtractReferenceMean(rpm::vector<double>& s, int n, int K);

static rpm::vector<double> downsampleSignal(const rpm::vector<double>& s, const double Fs, const double Fds);

static rpm::vector<double> calculateDownsampledNCCF(const rpm::vector<double>& dss, const int dsn, const int dsK1, const int dsK2);

static rpm::vector<std::pair<double, double>> findPeaksWithThreshold(const rpm::vector<double>& nccf, const double cand_tr, const int n_cands, const bool paraInterp);

static rpm::vector<double> calculateOriginalNCCF(const rpm::vector<double>& s, const double Fs, const double Fds, const int n, const int K, const rpm::vector<std::pair<double, double>>& dsPeaks);

static rpm::vector<RAPT::Cand> createCosts(const rpm::vector<std::pair<double, double>>& peaks, const double vo_bias, const double beta);

static double calculateRMS(const rpm::vector<double>& s, const int start, const int length);

static double expDistItakura(const rpm::vector<double>& ar1, const rpm::vector<double>& ar2);

static rpm::vector<double> lpcar2ra(const rpm::vector<double>& ar);
static rpm::vector<double> lpcar2rr(const rpm::vector<double>& ar);
static rpm::vector<double> lpcar2rf(const rpm::vector<double>& ar);
static rpm::vector<double> lpcrf2rr(const rpm::vector<double>& rf);

RAPT::RAPT()
    : lpcOrder(-1)
{
}

double RAPT::computeFrame(const double *data, int length, double Fs)
{
    if (lpcOrder < 0) {
        lpcOrder = 2 + std::round(Fs / 1000);
    }

    const double w = 0.0075;

    const int n = std::round(w * Fs);
    const int K = std::round(Fs / F0min);

    const double Fds = std::round(Fs / std::round(Fs / (4.0 * F0max)));
    const int dsn = std::round(w * Fds);
    const int dsK1 = std::round(Fds / F0max);
    const int dsK2 = std::round(Fds / F0min);

    const double beta = lag_wt / (Fs / F0min);
    
    const int J = std::round(0.03 * Fs);

    rpm::vector<double> s(data, data + length);
    
    if (s.size() < n + K) {
        s.resize(n + K, 0.0);
    }

    subtractReferenceMean(s, n, K);

    auto dss = downsampleSignal(s, Fs, Fds);
    
    if (dss.size() < dsn + dsK2) {
        dss.resize(dsn + dsK2, 0.0);
    }
    
    auto dsNCCF = calculateDownsampledNCCF(dss, dsn, dsK1, dsK2);
    auto dsPeaks = findPeaksWithThreshold(dsNCCF, cand_tr, n_cands, true);
   
    rpm::vector<std::pair<double, double>> peaks;
   
    rpm::vector<double> nccf;
    if (dsPeaks.size() > 0) {
        nccf = calculateOriginalNCCF(s, Fs, Fds, n, K, dsPeaks);
        peaks = findPeaksWithThreshold(nccf, cand_tr, n_cands, false);
    }

    if (frames.size() != nbFrames) {
        Frame defaultFrame = {
            { { vo_bias, false } },
            1e-10,
            1.0,
            1.0,
            {},
            -0.25,
        };
        frames.resize(nbFrames, defaultFrame);
    }

    Frame frm;
    frm.Fs = Fs;
    frm.cands = createCosts(peaks, vo_bias, beta);
  
    if (nbFrames > 1) {
        frm.rms = calculateRMS(s, length / 2 - J / 2, J);
        frm.rr = frm.rms / frames.back().rms;
    
        // Pre-emphasis.
        const double alpha = expf(-7000 / Fs);
        for (int i = (int) s.size() - 1; i >= 1; --i) {
            s[i] -= alpha * s[i - 1];
        }

        // Calculate AR.
        double gain;
        frm.ar = lpc.solve(s.data(), (int) s.size(), lpcOrder, &gain);
        frm.ar.insert(frm.ar.begin(), 1.0);
        frm.S = 0.2 / (expDistItakura(frm.ar, frames.back().ar) - 0.8);
    }

    double pitch = 0.0;
    
    if (frm.cands[0].voiced) {
        double Linterp = std::get<0>(parabolicInterpolation(nccf, frm.cands[0].L));
        pitch = frm.Fs / Linterp;
    }

    frames.pop_front();
    frames.push_back(std::move(frm));

    return pitch;
}

rpm::vector<double> RAPT::computePath()
{
    static rpm::vector<rpm::vector<rpm::vector<double>>> transitionMatrices;

    transitionMatrices.resize(nbFrames);

    for (int i = 1; i < nbFrames; ++i) {
        const int nj = (int) frames[i].cands.size();
        const int nk = (int) frames[i - 1].cands.size();

        auto& mat = transitionMatrices[i];
        mat.resize(nj);

        const double rri = frames[i].rr;
        const double Si = frames[i].S;
        
        for (int j = 0; j < nj; ++j) {
            auto& row = mat[j];
            row.resize(nk);
            
            const auto& cj = frames[i].cands[j];

            for (int k = 0; k < nk; ++k) {

                const auto& ck = frames[i - 1].cands[k];

                if (cj.voiced && ck.voiced) { // Voiced to voiced
                    double xi = fabsf(logf(cj.L / ck.L));
                    row[k] = freq_wt * std::min((double) xi, (doubl_c + fabs(xi - M_LN2)));
                }
                else if (!cj.voiced && ck.voiced) { // Voiced to unvoiced
                    row[k] = vtran_c + vtr_s_c * Si + vtr_a_c * rri;
                }
                else if (cj.voiced && !ck.voiced) { // Unvoiced to voiced
                    row[k] = vtran_c + vtr_s_c * Si + vtr_a_c / rri;
                }
                else { // Unvoiced to unvoiced
                    row[k] = 0.0;
                }
            }
        }
    }

    static rpm::vector<rpm::vector<double>> D;
    static rpm::vector<rpm::vector<int>> ks;
    
    D.resize(nbFrames + 1);
    D[0].resize(2, 0.0);

    ks.resize(nbFrames);

    for (int i = 0; i < nbFrames; ++i) {
        D[i + 1].resize(frames[i].cands.size());
        ks[i].resize(frames[i].cands.size());

        for (int j = 0; j < frames[i].cands.size(); ++j) {
            double min = HUGE_VALF;
            int kmin;
            for (int k = 0; k < D[i].size(); ++k) {
                double val = D[i][k] + i > 0 ? transitionMatrices[i][j][k] : 0.0;
                if (val < min) {
                    min = val;
                    kmin = k;
                }
            }
            
            D[i + 1][j] = frames[i].cands[j].localCost + min;
            ks[i][j] = kmin;
        }
    }

    rpm::vector<double> pitches(nbFrames);
 
    const int nj = (int) D[1].size();

    rpm::vector<int> indices(nj);
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(),
            [&](int j, int k) { return D[1][j] < D[1][k]; });

    int k = indices[0];

    pitches[0] = frames[0].cands[k].voiced ? frames[0].Fs / frames[0].cands[k].L : 0.0;

    for (int i = 1; i < nbFrames; ++i) {
        k = ks[i][k];
        
        pitches[i] = frames[i].cands[k].voiced ? frames[i].Fs / frames[i].cands[k].L : 0.0;
    }

    return pitches;
}

// utility functions.

void subtractReferenceMean(rpm::vector<double>& s, int n, int K)
{
    double mu = 0.0;
    for (int j = 0; j < n; ++j)
        mu += s[j];
    mu /= (double) n;
    for (int j = 0; j < s.size(); ++j) 
        s[j] -= mu;
}

rpm::vector<double> downsampleSignal(const rpm::vector<double>& s, const double Fs, const double Fds)
{
    size_t olen = s.size() * Fds / Fs + 0.5;
    
    rpm::vector<float> sFloat(s.begin(), s.end());
    rpm::vector<float> outFloat(olen);

    SRC_DATA data;
    data.data_in = sFloat.data();
    data.data_out = outFloat.data();
    data.input_frames = sFloat.size();
    data.output_frames = olen;
    data.src_ratio = (double) Fds / (double) Fs;

    int error = src_simple(&data, SRC_SINC_MEDIUM_QUALITY, 1);

    outFloat.resize(data.output_frames_gen);

    return rpm::vector<double>(outFloat.begin(), outFloat.end());
}

rpm::vector<double> calculateDownsampledNCCF(const rpm::vector<double>& dss, const int dsn, const int dsK1, const int dsK2)
{
    rpm::vector<double> dsNCCF(dsK2 + 1, 0.0);

    double dse0 = 0.0;
    for (int l = 0; l < dsn; ++l) {
        dse0 += dss[l] * dss[l];
    }

    for (int k = dsK1; k <= dsK2; ++k) {
        double p, q;
        p = q = 0.0;

        for (int j = 0; j < dsn; ++j) {
            p += dss[j] * dss[j + k];
        }

        for (int l = k; l < k + dsn; ++l) {
            q += dss[l] * dss[l];
        }

        q = sqrtf(dse0 * q);

        dsNCCF[k] = p / q;
    }

    return dsNCCF;
}

rpm::vector<std::pair<double, double>> findPeaksWithThreshold(const rpm::vector<double>& nccf, const double cand_tr, const int n_cands, const bool paraInterp)
{
    double max = -HUGE_VALF;
    for (int i = 0; i < nccf.size(); ++i) {
        if (nccf[i] > max) {
            max = nccf[i];
        }
    }

    double threshold = cand_tr * max;

    auto allPeaks = findPeaks(nccf.data(), (int) nccf.size());

    rpm::vector<std::pair<double, double>> peaks;

    for (const int& k : allPeaks) {
        if (k >= 0 && k < nccf.size() && nccf[k] > threshold) {
            if (paraInterp) {
                peaks.push_back(parabolicInterpolation(nccf, k));
            }
            else {
                peaks.push_back(std::make_pair(k, nccf[k]));
            }
        }
    }

    std::sort(peaks.begin(), peaks.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (peaks.size() > n_cands - 1) {
        peaks.erase(std::next(peaks.begin(), n_cands - 1), peaks.end());
    }

    return peaks;
}

rpm::vector<double> calculateOriginalNCCF(const rpm::vector<double>& s, const double Fs, const double Fds, const int n, const int K, const rpm::vector<std::pair<double, double>>& dsPeaks)
{
    rpm::set<int> lagsToCalculate;
    for (const auto& [dsk, y] : dsPeaks) {
        const int k = std::round((Fs * dsk) / Fds);
       
        if (k >= 0 && k <= K)
            lagsToCalculate.emplace(k);

        for (int l = 1; l <= 3; ++l) {
            if (k - l >= 0 && k - l <= K)
                lagsToCalculate.emplace(k - l);

            if (k + l >= 0 && k + l <= K)
                lagsToCalculate.emplace(k + l);
        }
    }
    
    rpm::vector<double> nccf(K + 1, 0.0);

    double e0;
    {
        int j = 0;
        double v = 0.0;
        for (int l = j; l < j + n; ++l) {
            v += s[l] * s[l];
        }
        e0 = v;
    }

    for (const int& k : lagsToCalculate) {
        double p, q;
        p = q = 0.0;

        for (int j = 0; j < n; ++j) {
            p += s[j] * s[j + k];
        }

        for (int l = k; l < k + n; ++l) {
            q += s[l] * s[l];
        }

        q = sqrtf(e0 * q);

        nccf[k] = p / q;
    }
    
    return nccf;
}

rpm::vector<RAPT::Cand> createCosts(const rpm::vector<std::pair<double, double>>& peaks, const double vo_bias, const double beta)
{
    rpm::vector<RAPT::Cand> costs(peaks.size() + 1);
    
    int i = 0;
    for (const auto& [k, y] : peaks) {
        costs[i] = {
            1.0 - y * (1.0 - beta * k),
            true,
            k,
            y,
        };
        i++;
    }

    double maxCij = 0.0;
    
    if (peaks.size() > 0) {
        maxCij = std::get<1>(
            *std::max_element(peaks.begin(), peaks.end(),
                [](auto& x, auto& y) { return x.second > y.second; }));
    }

    costs[i] = {
        vo_bias + maxCij,
        false
    };

    std::sort(costs.begin(), costs.end(),
            [](const auto& a, const auto& b) { return a.localCost < b.localCost; });

    return costs;
}

double calculateRMS(const rpm::vector<double>& s, const int start, const int length)
{
    double sum = 0.0;

    for (int i = 0; i < length; ++i) {
        double Wj = 0.5 - 0.5 * cos(2.0 * M_PI / (length - 1));
        double sj = s[start + i];

        double v = Wj * sj;

        sum += v * v;
    }

    return (double) sqrt(sum / length);
}

double expDistItakura(const rpm::vector<double>& ar1, const rpm::vector<double>& ar2)
{
    // d = 2 * sum ( lpcar2rr (ar1) * m2 ) * (ar1[0] / ar2[0]) ^ 2
    
    double denom = (ar1[0] * ar1[0]) / (ar2[0] * ar2[0]);

    auto m2 = lpcar2ra(ar2);
    m2[0] /= 2.0;
    
    auto rr1 = lpcar2rr(ar1);

    double numer = 0.0;

    for (int i = 0; i < m2.size(); ++i) {
        numer += m2[i] * rr1[i];
    }

    return 2.0 * numer / denom;
}

rpm::vector<double> lpcar2ra(const rpm::vector<double>& ar)
{
    const int p = (int) ar.size();
    
    rpm::vector<double> ra(p);

    for (int i = 0; i < p; ++i) {
        ra[i] = 0.0;
        for (int j = 0; j < p - i; ++j) {
            ra[i] += ar[j] * ar[i + j];
        }
    }

    return ra;
}

rpm::vector<double> lpcar2rr(const rpm::vector<double>& ar)
{
    double k = 1.0 / (ar[0] * ar[0]);

    if (ar.size() == 1) {
        return { k };
    }
    else {
        rpm::vector<double> rr = lpcrf2rr(lpcar2rf(ar));
        for (int i = 0; i < rr.size(); ++i) {
            rr[i] *= k;
        }
        return rr;
    }
}

rpm::vector<double> lpcar2rf(const rpm::vector<double>& ar)
{
    const int p = (int) ar.size();

    rpm::vector<double> rf = ar;
    
    for (int j = p - 2; j >= 1; --j) {
        double k = rf[j + 1];
        double d = 1.0 / (1.0 - k * k); 
        
        rpm::vector<double> temp = rf;
        for (int i = 1; i <= j; ++i) {
            temp[i] = (rf[i] - k * rf[j + 1 - i]) * d;
        }
        rf = std::move(temp);
    }

    return rf;
}

rpm::vector<double> lpcrf2rr(const rpm::vector<double>& rf)
{
    const int p1 = (int) rf.size();
    const int p0 = p1 - 1;

    if (p0 > 0) {
        rpm::vector<double> a;
        a.push_back(rf[1]);

        rpm::vector<double> rr(p1, 0.0);
        rr[0] = 1.0;
        rr[1] = -a[0];

        double e = a[0] * a[0] - 1.0;
        for (int n = 1; n < p0; ++n) {
            double k = rf[n + 1];
            rr[n + 1] = k * e;
            for (int j = n - 1; j >= 1; --j) {
                rr[n + 1] -= rr[j] * a[n - 1 - j];
            }
            for (int j = 0; j < a.size(); ++j) {
                a[j] += k * a[n - 1 - j];
            }
            a.push_back(k);
            e *= (1.0 - k * k);
        }
        
        double r0 = rr[0];
        for (int i = 1; i < a.size(); ++i) {
            r0 += rr[i] * a[i - 1];
        }
        r0 = 1.0 / r0;

        for (int i = 0; i < rr.size(); ++i) {
            rr[i] *= r0;
        }

        return rr;
    }
    else {
        return { 1.0 };
    }
}
