#include "sigma.h"
#include "../wavelet/wavelet.h"
#include <list>
#include <armadillo>
#include <iostream>

std::vector<double> SIGMA::analyse(const std::vector<double>& lx, double fs)
{
    constexpr double fmin = 50;
    constexpr double fmax = 400;

    constexpr double Tmin = 1.0 / fmax;
    constexpr double Tmax = 1.0 / fmin;

    constexpr double oqmin = 0.1;
    constexpr double oqmax = 0.9;

    constexpr double gwlen = Tmin;
    constexpr double fwlen = 0.000;
    
    constexpr int nclust = 2;
   
    // Calculate SWT
    constexpr int nlev = 5;
    Wt::DiscreteWavelet wavelet(Wt::BIOR, 15);
    std::vector<double> swc = Analysis::swt(lx, wavelet, nlev, 0, true);

    // Calculate multiscale product
    int dlen = Wt::swt_buffer_length(lx.size());
    std::vector<double> mp(dlen, 1.0);
    for (int k = 1; k * dlen < (int) swc.size(); ++k) {
        for (int i = 0; i < dlen; ++i) {
            mp[i] *= swc[k * dlen + i];
        }
    }
    
    // Find third roots
    std::vector<double> nmp(mp);
    std::vector<double> pmp(mp);
    std::vector<double> crnmp(mp.size());
    std::vector<double> crpmp(mp.size());
    for (int i = 0; i < (int) mp.size(); ++i) {
        // Half-wave rectify on negative half of mp for GCI
        if (nmp[i] > 0)
            nmp[i] = 0;

        // Half-wave rectify on positive half of mp for GOI
        if (pmp[i] < 0)
            pmp[i] = 0;

        crnmp[i] = cbrt(nmp[i]);
        crpmp[i] = cbrt(pmp[i]);
    }

    // Group delay evaluation on mp
    auto [gcic, nsew, ngrdel, ntoff] = xewgrdel(nmp, fs, gwlen, fwlen);
    ngrdel.insert(ngrdel.begin(), ntoff, 0.0);
    ngrdel.erase(std::prev(ngrdel.end(), ntoff), ngrdel.end());

    auto [goic, psew, pgrdel, ptoff] = xewgrdel(pmp, fs, gwlen, fwlen);
    pgrdel.insert(pgrdel.begin(), ptoff, 0.0);
    pgrdel.erase(std::prev(pgrdel.end(), ptoff), pgrdel.end());

    // Set up other variables
    std::vector<double> gci(lx.size(), 0.0);
    std::vector<double> goi(lx.size(), 0.0);
    
    // --- GCI Detection ---

    // Model GD slope
    const int nr = (gwlen * fs) / 2 - 1;
    const int mngrdellen = 2 * nr + 1;
    std::vector<double> mngrdel(mngrdellen);
    for (int i = 0; i < mngrdellen; ++i) {
        mngrdel[i] = i - nr;
    }
    std::vector<double> cmngrdel(ngrdel.size(), 0.0);

    const int snfv = gcic.size();
    arma::mat nfv(3, snfv, arma::fill::zeros);

    for (int i = 0; i < snfv; ++i) {
        int lbnd = std::round(gcic[i] - nr);
        int ubnd = lbnd + mngrdellen - 1;

        if (lbnd >= 0 && ubnd < (int) ngrdel.size()) {
            double sum = 0.0;
            double min = HUGE_VAL;
            double mean = 0.0;

            for (int k = lbnd; k <= ubnd; ++k) {
                sum += crnmp[k];
                if (crnmp[k] < min)
                    min = crnmp[k];
                
                double v = mngrdel[k - lbnd] - ngrdel[k];
                mean += v * v;

                cmngrdel[k] = mngrdel[k - lbnd];
            }

            nfv(0, i) = sum; // Sum of crnmp over GD window
            nfv(1, i) = min; // Peak value of crnmp
            nfv(2, i) = sqrt(mean / mngrdellen); // Phase slope deviation
        }
    }

    // Determine clusters
    arma::gmm_diag ngmm;
    bool status = ngmm.learn(nfv, 3, arma::eucl_dist, arma::random_subset, 10, 5, 1e-8, false);

    // Find cluster with lowest crnmp sum
    int I = arma::index_min(ngmm.means.row(0));
   
    // If the candidate belongs to the chosen cluster then keep 
    for (int i = 0; i < snfv; ++i) {
        if (ngmm.assign(nfv.col(i), arma::eucl_dist) == (arma::uword) I) {
            int k = std::max(0, std::min((int) gci.size() - 1, (int) std::round(gcic[i])));
            gci[k] = 1.0;
        }
    }

    // --- Post-filter swallows (GCIs only) ---

    if (gci.size() > 2) {
        // If a gci is separated from all others by more than Tmax, delete
        std::list<int> fgci;
        for (int i = 0; i < (int) gci.size(); ++i) {
            if (gci[i] > 0) {
                fgci.push_back(i);
            }
        }
        
        // Check first one
        auto i0 = fgci.begin();
        auto i1 = std::next(i0);
        if (*i1 - *i0 > Tmax * fs) {
            fgci.pop_front();
        }

        // Check the middle
        auto i2 = std::next(fgci.begin(), 2);
        auto iend = std::prev(fgci.end());
        
        while (i2 != iend) {
            auto i2prev = std::prev(i2);
            auto i2next = std::next(i2);

            if ((*i2 - *i2prev > Tmax * fs)
                    && (*i2next - *i2 > Tmax * fs)) {
                fgci.erase(i2);
            }

            i2 = i2next;
        }

        // Check last one
        auto iendprev = std::prev(iend);
        if (*iend - *iendprev > Tmax * fs) {
            fgci.pop_back();
        }

        // Convert back
        std::fill(gci.begin(), gci.end(), 0.0);
        for (auto t : fgci) {
            gci[t] = 1.0;
        }
    }

    return gci;

    // --- GOI detection ---

}
