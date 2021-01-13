#include "../util/util.h"
#include "../filter/filter.h"
#include "../../modules/math/constants.h"
#include "rapt.h"
#include "pitch.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <set>

using namespace Analysis;

Pitch::RAPT::RAPT()
{
    F0min   = 70;
    F0max   = 500;
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

    if (f0 != 0.0f) {
        return {
            .pitch = f0,
            .voiced = true,
        };
    }
    else {
        return {
            .pitch = 0.0f,
            .voiced = false,
        };
    }
}

static void subtractReferenceMean(std::vector<double>& s, int n, int K);

static std::vector<double> downsampleSignal(const std::vector<double>& s, const double Fs, const double Fds, SpeexResamplerState **resampler);

static std::vector<double> calculateDownsampledNCCF(const std::vector<double>& dss, const int dsn, const int dsK1, const int dsK2);

static std::vector<std::pair<double, double>> findPeaksWithThreshold(const std::vector<double>& nccf, const double cand_tr, const int n_cands, const bool paraInterp);

static std::vector<double> calculateOriginalNCCF(const std::vector<double>& s, const double Fs, const double Fds, const int n, const int K, const std::vector<std::pair<double, double>>& dsPeaks);

static std::vector<RAPT::Cand> createCosts(const std::vector<std::pair<double, double>>& peaks, const double vo_bias, const double beta);

static double calculateRMS(const std::vector<double>& s, const int start, const int length);

static double expDistItakura(const std::vector<double>& ar1, const std::vector<double>& ar2);

static std::vector<double> lpcar2ra(const std::vector<double>& ar);
static std::vector<double> lpcar2rr(const std::vector<double>& ar);
static std::vector<double> lpcar2rf(const std::vector<double>& ar);
static std::vector<double> lpcrf2rr(const std::vector<double>& rf);

RAPT::RAPT()
    : lpcOrder(-1), resampler(nullptr)
{
}

RAPT::~RAPT()
{
    if (resampler != nullptr) {
        speex_resampler_destroy(resampler);
    }
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
    
    const int J = std::round(0.03f * Fs);

    std::vector<double> s(data, data + length);
    
    if (s.size() < n + K) {
        s.resize(n + K, 0.0f);
    }

    subtractReferenceMean(s, n, K);

    auto dss = downsampleSignal(s, Fs, Fds, &resampler);
    
    if (dss.size() < dsn + dsK2) {
        dss.resize(dsn + dsK2, 0.0f);
    }
    
    auto dsNCCF = calculateDownsampledNCCF(dss, dsn, dsK1, dsK2);
    auto dsPeaks = findPeaksWithThreshold(dsNCCF, cand_tr, n_cands, true);
   
    std::vector<std::pair<double, double>> peaks;
   
    std::vector<double> nccf;
    if (dsPeaks.size() > 0) {
        nccf = calculateOriginalNCCF(s, Fs, Fds, n, K, dsPeaks);
        peaks = findPeaksWithThreshold(nccf, cand_tr, n_cands, false);
    }

    if (frames.size() != nbFrames) {
        Frame defaultFrame = {
            .cands = { { .localCost = vo_bias, .voiced = false } },
            .rms = 1e-10f,
            .rr = 1.0f,
            .ar = { 1.0f },
            .S = -0.25f,
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
        for (int i = s.size() - 1; i >= 1; --i) {
            s[i] -= alpha * s[i - 1];
        }

        // Calculate AR.
        double gain;
        frm.ar = lpc.solve(s.data(), s.size(), lpcOrder, &gain);
        frm.ar.insert(frm.ar.begin(), 1.0f);
        frm.S = 0.2f / (expDistItakura(frm.ar, frames.back().ar) - 0.8f);
    }

    double pitch = 0.0f;
    
    if (frm.cands[0].voiced) {
        double Linterp = std::get<0>(parabolicInterpolation(nccf, frm.cands[0].L));
        pitch = frm.Fs / Linterp;
    }

    frames.pop_front();
    frames.push_back(std::move(frm));

    return pitch;
}

std::vector<double> RAPT::computePath()
{
    static std::vector<std::vector<std::vector<double>>> transitionMatrices;

    transitionMatrices.resize(nbFrames);

    for (int i = 1; i < nbFrames; ++i) {
        const int nj = frames[i].cands.size();
        const int nk = frames[i - 1].cands.size();

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
                    row[k] = 0.0f;
                }
            }
        }
    }

    static std::vector<std::vector<double>> D;
    static std::vector<std::vector<int>> ks;
    
    D.resize(nbFrames + 1);
    D[0].resize(2, 0.0f);

    ks.resize(nbFrames);

    for (int i = 0; i < nbFrames; ++i) {
        D[i + 1].resize(frames[i].cands.size());
        ks[i].resize(frames[i].cands.size());

        for (int j = 0; j < frames[i].cands.size(); ++j) {
            double min = HUGE_VALF;
            int kmin;
            for (int k = 0; k < D[i].size(); ++k) {
                double val = D[i][k] + i > 0 ? transitionMatrices[i][j][k] : 0.0f;
                if (val < min) {
                    min = val;
                    kmin = k;
                }
            }
            
            D[i + 1][j] = frames[i].cands[j].localCost + min;
            ks[i][j] = kmin;
        }
    }

    std::vector<double> pitches(nbFrames);
 
    const int nj = D[1].size();

    std::vector<int> indices(nj);
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(),
            [&](int j, int k) { return D[1][j] < D[1][k]; });

    int k = indices[0];

    pitches[0] = frames[0].cands[k].voiced ? frames[0].Fs / frames[0].cands[k].L : 0.0f;

    for (int i = 1; i < nbFrames; ++i) {
        k = ks[i][k];
        
        pitches[i] = frames[i].cands[k].voiced ? frames[i].Fs / frames[i].cands[k].L : 0.0f;
    }

    return pitches;
}

// utility functions.

void subtractReferenceMean(std::vector<double>& s, int n, int K)
{
    double mu = 0.0f;
    for (int j = 0; j < n; ++j)
        mu += s[j];
    mu /= (double) n;
    for (int j = 0; j < s.size(); ++j) 
        s[j] -= mu;
}

std::vector<double> downsampleSignal(const std::vector<double>& s, const double Fs, const double Fds, SpeexResamplerState **resampler)
{
    static double lastFs(0), lastFds(0);

    if (*resampler == nullptr || lastFs != Fs || lastFds != Fds) {
        lastFs = Fs;
        lastFds = Fds;
        if (*resampler != nullptr) {
            speex_resampler_destroy(*resampler);
        }
        *resampler = speex_resampler_init(1, Fs, Fds, 9, nullptr);
    }
  
    speex_resampler_reset_mem(*resampler);
    speex_resampler_skip_zeros(*resampler);

    int latency = speex_resampler_get_input_latency(*resampler);

    spx_uint32_t inLen = s.size() + latency;
    spx_uint32_t outLen = (inLen * Fds) / Fs;
    
    std::vector<float> fin(inLen);
    std::vector<float> fout(outLen);
    std::copy(s.begin(), s.end(), fin.begin());

    speex_resampler_process_float(*resampler, 0, fin.data(), &inLen, fout.data(), &outLen);
    fout.resize(outLen);

    return std::vector<double>(fout.begin(), fout.end());
}

std::vector<double> calculateDownsampledNCCF(const std::vector<double>& dss, const int dsn, const int dsK1, const int dsK2)
{
    std::vector<double> dsNCCF(dsK2 + 1, 0.0f);

    double dse0;
    for (int l = 0; l < dsn; ++l) {
        dse0 += dss[l] * dss[l];
    }

    for (int k = dsK1; k <= dsK2; ++k) {
        double p, q;
        p = q = 0.0f;

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

std::vector<std::pair<double, double>> findPeaksWithThreshold(const std::vector<double>& nccf, const double cand_tr, const int n_cands, const bool paraInterp)
{
    double max = -HUGE_VALF;
    for (int i = 0; i < nccf.size(); ++i) {
        if (nccf[i] > max) {
            max = nccf[i];
        }
    }

    double threshold = cand_tr * max;

    auto allPeaks = findPeaks(nccf.data(), nccf.size());

    std::vector<std::pair<double, double>> peaks;

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

std::vector<double> calculateOriginalNCCF(const std::vector<double>& s, const double Fs, const double Fds, const int n, const int K, const std::vector<std::pair<double, double>>& dsPeaks)
{
    std::set<int> lagsToCalculate;
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
    
    std::vector<double> nccf(K + 1, 0.0f);

    double e0;
    {
        int j = 0;
        double v = 0.0f;
        for (int l = j; l < j + n; ++l) {
            v += s[l] * s[l];
        }
        e0 = v;
    }

    for (const int& k : lagsToCalculate) {
        double p, q;
        p = q = 0.0f;

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

std::vector<RAPT::Cand> createCosts(const std::vector<std::pair<double, double>>& peaks, const double vo_bias, const double beta)
{
    std::vector<RAPT::Cand> costs(peaks.size() + 1);
    
    int i = 0;
    for (const auto& [k, y] : peaks) {
        costs[i] = {
            .localCost = 1.0f - y * (1.0f - beta * k),
            .voiced = true,
            .L = k,
            .C = y,
        };
        i++;
    }

    double maxCij = 0.0f;
    
    if (peaks.size() > 0) {
        maxCij = std::get<1>(
            *std::max_element(peaks.begin(), peaks.end(),
                [](auto& x, auto& y) { return x.second > y.second; }));
    }

    costs[i] = {
        .localCost = vo_bias + maxCij,
        .voiced = false
    };

    std::sort(costs.begin(), costs.end(),
            [](const auto& a, const auto& b) { return a.localCost < b.localCost; });

    return costs;
}

double calculateRMS(const std::vector<double>& s, const int start, const int length)
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

double expDistItakura(const std::vector<double>& ar1, const std::vector<double>& ar2)
{
    // d = 2 * sum ( lpcar2rr (ar1) * m2 ) * (ar1[0] / ar2[0]) ^ 2
    
    double denom = (ar1[0] * ar1[0]) / (ar2[0] * ar2[0]);

    auto m2 = lpcar2ra(ar2);
    m2[0] /= 2.0f;
    
    auto rr1 = lpcar2rr(ar1);

    double numer = 0.0f;

    for (int i = 0; i < m2.size(); ++i) {
        numer += m2[i] * rr1[i];
    }

    return 2.0f * numer / denom;
}

std::vector<double> lpcar2ra(const std::vector<double>& ar)
{
    const int p = ar.size();
    
    std::vector<double> ra(p);

    for (int i = 0; i < p; ++i) {
        ra[i] = 0.0f;
        for (int j = 0; j < p - i; ++j) {
            ra[i] += ar[j] * ar[i + j];
        }
    }

    return ra;
}

std::vector<double> lpcar2rr(const std::vector<double>& ar)
{
    double k = 1.0f / (ar[0] * ar[0]);

    if (ar.size() == 1) {
        return { k };
    }
    else {
        std::vector<double> rr = lpcrf2rr(lpcar2rf(ar));
        for (int i = 0; i < rr.size(); ++i) {
            rr[i] *= k;
        }
        return rr;
    }
}

std::vector<double> lpcar2rf(const std::vector<double>& ar)
{
    const int p = ar.size();

    std::vector<double> rf = ar;
    
    for (int j = p - 2; j >= 1; --j) {
        double k = rf[j + 1];
        double d = 1.0f / (1.0f - k * k); 
        
        std::vector<double> temp = rf;
        for (int i = 1; i <= j; ++i) {
            temp[i] = (rf[i] - k * rf[j + 1 - i]) * d;
        }
        rf = std::move(temp);
    }

    return rf;
}

std::vector<double> lpcrf2rr(const std::vector<double>& rf)
{
    const int p1 = rf.size();
    const int p0 = p1 - 1;

    if (p0 > 0) {
        std::vector<double> a;
        a.push_back(rf[1]);

        std::vector<double> rr(p1, 0.0f);
        rr[0] = 1.0f;
        rr[1] = -a[0];

        double e = a[0] * a[0] - 1.0f;
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
            e *= (1.0f - k * k);
        }
        
        double r0 = rr[0];
        for (int i = 1; i < a.size(); ++i) {
            r0 += rr[i] * a[i - 1];
        }
        r0 = 1.0f / r0;

        for (int i = 0; i < rr.size(); ++i) {
            rr[i] *= r0;
        }

        return rr;
    }
    else {
        return { 1.0f };
    }
}
