#ifndef ANALYSIS_WAVELETS_H
#define ANALYSIS_WAVELETS_H

#include "rpcxx.h"
#include <string>

namespace Wt {

    enum Symmetry {
        UNKNOWN = -1,
        ASYMMETRIC = 0,
        NEAR_SYMMETRIC = 1,
        SYMMETRIC = 2,
        ANTI_SYMMETRIC = 3,
    };

    enum WaveletName {
        HAAR,
        RBIO,
        DB,
        SYM,
        COIF,
        BIOR,
        DMEY,
        GAUS,
        MEXH,
        MORL,
        CGAU,
        SHAN,
        FBSP,
        CMOR,
    };

    struct BaseWavelet {
        int support_width;
        Symmetry symmetry;
        bool orthogonal;
        bool biorthogonal;
        bool compact_support;
        std::string family_name;
        std::string short_name;
    };

    struct DiscreteWavelet : public BaseWavelet {
        rpm::vector<double> dec_hi;
        rpm::vector<double> dec_lo;
        int dec_len;
        rpm::vector<double> rec_hi;
        rpm::vector<double> rec_lo;
        int rec_len;
        int vanishing_moments_psi;
        int vanishing_moments_phi;

        DiscreteWavelet(WaveletName name, int order) { init(name, order); }

    private:
        void init(WaveletName name, int order);
        void init(int filters_length);
    };

    struct ContinuousWavelet : public BaseWavelet {
        double lower_bound;
        double upper_bound;
        bool complex_cwt;
        double center_frequency;
        double bandwidth_frequency;
        int fbsp_order;

        ContinuousWavelet(WaveletName name, int order);
    };

}

#endif // ANALYSIS_WAVELETS_H
