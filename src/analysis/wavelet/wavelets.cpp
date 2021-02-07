#include "wavelet.h"
#include "coeffs.h"
#include <algorithm>
#include <stdexcept>

void Wt::DiscreteWavelet::init(int filters_length)
{
    if (filters_length > 0 && filters_length % 2 == 1) {
        filters_length++;
    }

    dec_len = rec_len = filters_length;
    dec_lo.resize(dec_len, 0.0);
    dec_hi.resize(dec_len, 0.0);
    rec_lo.resize(rec_len, 0.0);
    rec_hi.resize(rec_len, 0.0);

    support_width = -1;
    orthogonal = false;
    biorthogonal = false;
    symmetry = UNKNOWN;
    compact_support = false;
    family_name = "";
    short_name = "";
    vanishing_moments_psi = 0;
    vanishing_moments_phi = 0;
}

void Wt::DiscreteWavelet::init(WaveletName name, int order)
{
    if (name == HAAR) {
        init(DB, 1);
        family_name = "Haar";
        short_name = "haar";
        return;
    }
    else if (name == RBIO) {
        init(BIOR, order);

        std::swap(dec_len, rec_len);
        std::swap(rec_lo, dec_lo);
        std::swap(rec_hi, dec_hi);

        std::reverse(rec_lo.begin(), rec_lo.end());
        std::reverse(rec_hi.begin(), rec_hi.end());
        std::reverse(dec_lo.begin(), dec_lo.end());
        std::reverse(dec_hi.begin(), dec_hi.end());

        family_name = "Reverse biorthogonal";
        short_name = "rbio";

        return;
    }

    switch (name) {
        /* Daubechies wavelets family */
        case DB: {
            int coeffs_idx = order - 1;
            if (coeffs_idx >= NELEMS(db_double))
                goto err;
            
            init(2 * order);
            vanishing_moments_psi = order;
            vanishing_moments_phi = 0;
            support_width = 2 * order - 1;
            orthogonal = true;
            biorthogonal = true;
            symmetry = ASYMMETRIC;
            compact_support = true;
            family_name = "Daubechies";
            short_name = "db";
            
            size_t i;
            for(i = 0; i < rec_len; ++i){
                rec_lo[i] = db_double[coeffs_idx][i];
                dec_lo[i] = db_double[coeffs_idx][dec_len-1-i];
                rec_hi[i] = ((i % 2) ? -1 : 1)
                  * db_double[coeffs_idx][dec_len-1-i];
                dec_hi[i] = (((dec_len-1-i) % 2) ? -1 : 1)
                  * db_double[coeffs_idx][i];
            }
       
            break;
        }

        /* Symlets wavelets family */
        case SYM: {
            int coeffs_idx = order - 2;
            if (coeffs_idx >= NELEMS(sym_double))
                goto err;

            init(2 * order);
            vanishing_moments_psi = order;
            vanishing_moments_phi = 0;
            support_width = 2 * order - 1;
            orthogonal = true;
            biorthogonal = true;
            symmetry = NEAR_SYMMETRIC;
            compact_support = true;
            family_name = "Symlets";
            short_name = "sym";

            size_t i;
            for(i = 0; i < rec_len; ++i){
                rec_lo[i] = sym_double[coeffs_idx][i];
                dec_lo[i] = sym_double[coeffs_idx][dec_len-1-i];
                rec_hi[i] = ((i % 2) ? -1 : 1)
                  * sym_double[coeffs_idx][dec_len-1-i];
                dec_hi[i] = (((dec_len-1-i) % 2) ? -1 : 1)
                  * sym_double[coeffs_idx][i];
            }

            break;
        }

        /* Coiflets wavelets family */
        case COIF: {
            int coeffs_idx = order - 1;
            if (coeffs_idx >= NELEMS(coif_double))
                goto err;

            init(6 * order);
            vanishing_moments_psi = 2 * order;
            vanishing_moments_phi = 2 * order - 1;
            support_width = 6 * order - 1;
            orthogonal = true;
            biorthogonal = true;
            symmetry = NEAR_SYMMETRIC;
            compact_support = true;
            family_name = "Coiflets";
            short_name = "coif";

            size_t i;
            for(i = 0; i < rec_len; ++i){
                rec_lo[i] = coif_double[coeffs_idx][i] * M_SQRT2;
                dec_lo[i] = coif_double[coeffs_idx][dec_len-1-i]
                  * M_SQRT2;
                rec_hi[i] = ((i % 2) ? -1 : 1)
                  * coif_double[coeffs_idx][dec_len-1-i] * M_SQRT2;
                dec_hi[i] = (((dec_len-1-i) % 2) ? -1 : 1)
                  * coif_double[coeffs_idx][i] * M_SQRT2;
            }

            break;
        }

        /* Biorthogonal wavelets family */
        case BIOR: {
            int N = order / 10;
            int M = order % 10;

            int M_idx, M_max;
            switch (N) {
                case 1:
                    if (M % 2 != 1 || M > 5) goto err;
                    M_idx = M / 2;
                    M_max = 5;
                    break;
                case 2:
                    if (M % 2 != 0 || M < 2 || M > 8) goto err;
                    M_idx = M / 2 - 1;
                    M_max = 8;
                    break;
                case 3:
                    if (M % 2 != 1) goto err;
                    M_idx = M / 2;
                    M_max = 9;
                    break;
                case 4:
                case 5:
                    if (M != N) goto err;
                    M_idx = 0;
                    M_max = M;
                    break;
                case 6:
                    if (M != 8) goto err;
                    M_idx = 0;
                    M_max = 8;
                    break;
                default:
                    goto err;
            }
          
            init((N == 1) ? 2 * M : 2 * M + 2);
            vanishing_moments_psi = N;
            vanishing_moments_phi = M;
            support_width = -1;
            orthogonal = false;
            biorthogonal = true;
            symmetry = SYMMETRIC;
            compact_support = true;
            family_name = "Biorthogonal";
            short_name = "bior";

            size_t n = M_max - M;
            size_t i;
            for(i = 0; i < rec_len; ++i){
                rec_lo[i] = bior_double[N - 1][0][i+n];
                dec_lo[i] = bior_double[N - 1][M_idx+1][dec_len-1-i];
                rec_hi[i] = ((i % 2) ? -1 : 1)
                  * bior_double[N - 1][M_idx+1][dec_len-1-i];
                dec_hi[i] = (((dec_len-1-i) % 2) ? -1 : 1)
                  * bior_double[N - 1][0][i+n];
            }

            break; 
        }

        /* Discrete FIR filter approximation of Meyer wavelet */
        case DMEY: {
            init(62);
            vanishing_moments_psi = -1;
            vanishing_moments_phi = -1;
            support_width = -1;
            orthogonal = true;
            biorthogonal = true;
            symmetry = SYMMETRIC;
            compact_support = true;
            family_name = "Discrete Meyer (FIR Approximation)";
            short_name = "dmey";
            
            size_t i;
            for(i = 0; i < rec_len; ++i){
                rec_lo[i] = dmey_double[i];
                dec_lo[i] = dmey_double[dec_len-1-i];
                rec_hi[i] = ((i % 2) ? -1 : 1)
                  * dmey_double[dec_len-1-i];
                dec_hi[i] = (((dec_len-1-i) % 2) ? -1 : 1)
                  * dmey_double[i];
            }

            break;
        }

        default:
            throw std::runtime_error("Wavelet name invalid");
    }

    return;

err:
    throw std::runtime_error("Wavelet order too high");
}

Wt::ContinuousWavelet::ContinuousWavelet(WaveletName name, int order)
{
    switch (name) {
        case GAUS:
            if (order > 8) goto err;
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            if (order % 2 == 0)
                symmetry = SYMMETRIC;
            else
                symmetry = ANTI_SYMMETRIC;
            compact_support = false;
            family_name = "Gaussian wavelet";
            short_name = "gaus";
            complex_cwt = false;
            lower_bound = -5;
            upper_bound = 5;
            center_frequency = 0;
            bandwidth_frequency = 0;
            fbsp_order = 0;
            break;
        case MEXH:
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            symmetry = SYMMETRIC;
            compact_support = false;
            family_name = "Mexican hat wavelet";
            short_name = "mexh";
            complex_cwt = false;
            lower_bound = -8;
            upper_bound = 8;
            center_frequency = 0;
            bandwidth_frequency = 0;
            fbsp_order = 0;
            break;
        case MORL:
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            symmetry = SYMMETRIC;
            compact_support = false;
            family_name = "Morlet wavelet";
            short_name = "morl";
            complex_cwt = false;
            lower_bound = -8;
            upper_bound = 8;
            center_frequency = 0;
            bandwidth_frequency = 0;
            fbsp_order = 0;
            break;
        case CGAU:
            if (order > 8) goto err;
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            if (order % 2 == 0)
                symmetry = SYMMETRIC;
            else
                symmetry = ANTI_SYMMETRIC;
            compact_support = false;
            family_name = "Complex Gaussian  wavelet";
            short_name = "cgau";
            complex_cwt = true;
            lower_bound = -5;
            upper_bound = 5;
            center_frequency = 0;
            bandwidth_frequency = 0;
            fbsp_order = 0;
            break;
        case SHAN:
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            symmetry = ASYMMETRIC;
            compact_support = false;
            family_name = "Shannon wavelet";
            short_name = "shan";
            complex_cwt = true;
            lower_bound = -20;
            upper_bound = 20;
            center_frequency = 1;
            bandwidth_frequency = 0.5;
            fbsp_order = 0;
            break;
        case FBSP:
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            symmetry = ASYMMETRIC;
            compact_support = false;
            family_name = "Frequency B-Spline wavelet";
            short_name = "fbsp";
            complex_cwt = true;
            lower_bound = -20;
            upper_bound = 20;
            center_frequency = 0.5;
            bandwidth_frequency = 1;
            fbsp_order = 2;
            break;
        case CMOR:
            support_width = -1;
            orthogonal = false;
            biorthogonal = false;
            symmetry = ASYMMETRIC;
            compact_support = false;
            family_name = "Complex Morlet wavelet";
            short_name = "cmor";
            complex_cwt = true;
            lower_bound = -8;
            upper_bound = 8;
            center_frequency = 0.5;
            bandwidth_frequency = 1;
            fbsp_order = 0;
            break;
        default:
            throw std::runtime_error("Wavelet name invalid");
    }

    return;

err:
    throw std::runtime_error("Wavelet order too high");
}

