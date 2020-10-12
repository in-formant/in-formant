#include "filter.h"
#include <algorithm>
#include <iostream>

static std::pair<std::vector<std::complex<float>>, std::vector<float>> cplxreal(const std::vector<std::complex<float>>& z_)
{
    float tol = 100.0f * std::numeric_limits<float>::epsilon();
    
    std::vector<std::complex<float>> z(z_.begin(), z_.end());
    std::sort(z.begin(), z.end(), [](auto& x, auto& y) { return x.real() < y.real(); });

    std::vector<std::complex<float>> cplx;
    std::vector<float> real;

    for (const auto& x : z) {
        if (fabsf(x.imag()) / (std::abs(x) + std::numeric_limits<float>::min()) <= tol) {
            real.push_back(x.real());
        }
        else {
            cplx.push_back(x);
        }
    }

    const int m = cplx.size();
    std::vector<std::complex<float>> zc;

    for (int i = 0; i < m; i += 2) {
        float v = std::numeric_limits<float>::max();
        int idx;
        for (int k = i + 1; k < m; ++k) {
            float vk = std::abs(z[k] - std::conj(z[i]));
            if (vk < v) {
                v = vk;
                idx = k;
            }
        }
        if (v >= tol * std::abs(z[i])) {
            std::cerr << "cplxreal: Could not pair all complex numbers" << std::endl;
            break;
        }
        if (z[i].imag() < 0) {
            std::swap(z[i], z[idx]);
        }
        std::swap(z[idx], z[i + 1]);
        zc.push_back(z[i]);
    }
    
    return { std::move(zc), std::move(real) };
}

std::vector<std::array<float, 6>> Analysis::zpk2sos(
        const std::vector<std::complex<float>>& z,
        const std::vector<std::complex<float>>& p,
        float k)
{
    auto [zc, zr] = cplxreal(z);
    auto [pc, pr] = cplxreal(p);

    const int nzc = zc.size();
    const int npc = pc.size();

    int nzr = zr.size();
    int npr = pr.size();

    // Pair up real zeroes.
    int nzrsec;
    std::vector<float> zrms, zrp;
    if (nzr > 0) {
        if (nzr % 2 == 1) {
            zr.push_back(0.0f);
            nzr++;
        }
        nzrsec = nzr / 2;
        zrms.resize(nzrsec);
        zrp.resize(nzrsec);
        for (int i = 0; i < nzr - 1; i += 2) {
            zrms[i / 2] = -zr[i] - zr[i + 1];
            zrp[i / 2]  =  zr[i] * zr[i + 1];
        }
    }
    else {
        nzrsec = 0;
    }

    // Pair up real poles.
    int nprsec;
    std::vector<float> prms, prp;
    if (npr > 0) {
        if (npr % 2 == 1) {
            pr.push_back(0.0f);
            npr++;
        }
        nprsec = npr / 2;
        prms.resize(nprsec);
        prp.resize(nprsec);
        for (int i = 0; i < npr - 1; i += 2) {
            prms[i / 2] = -pr[i] - pr[i + 1];
            prp[i / 2]  =  pr[i] * pr[i + 1];
        }
    }
    else {
        nprsec = 0;
    }

    const int nsecs = std::max(nzc + nzrsec, npc + nprsec);

    std::vector<float> zcm2r(nzc), zca2(nzc);
    for (int i = 0; i < nzc; ++i) {
        zcm2r[i] = -2.0f * zc[i].real();
        float a = std::abs(zc[i]);
        zca2[i] = a * a;
    }

    std::vector<float> pcm2r(npc), pca2(npc);
    for (int i = 0; i < npc; ++i) {
        pcm2r[i] = -2.0f * pc[i].real();
        float a = std::abs(pc[i]);
        pca2[i] = a * a;
    }

    std::vector<std::array<float, 6>> sos(nsecs);
    
    for (int i = 0; i < nsecs; ++i) {
        sos[i][0] = 1.0f;
        sos[i][3] = 1.0f;
    }

    const int nzrl = nzc + nzrsec;
    const int nprl = npc + nprsec;

    for (int i = 0; i < nsecs; ++i) {
        if (i < nzc) {
            sos[i][1] = zcm2r[i];
            sos[i][2] = zca2[i];
        }
        else if (i < nzrl) {
            sos[i][1] = zrms[i - nzc];
            sos[i][2] = zrp[i - nzc];
        }
        else {
            sos[i][1] = sos[i][2] = 0.0f;
        }

        if (i < npc) {
            sos[i][4] = pcm2r[i];
            sos[i][5] = pca2[i];
        }
        else if (i < nprl) {
            sos[i][4] = prms[i - npc];
            sos[i][5] = prp[i - npc];
        }
        else {
            sos[i][4] = sos[i][5] = 0.0f;
        }
    }

    if (sos.size() > 0) {
        for (int i = 0; i < 3; ++i) {
            sos[0][i] *= k;
        }
    }

    return sos;
}

