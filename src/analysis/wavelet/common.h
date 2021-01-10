#ifndef WAVELET_COMMON_H
#define WAVELET_COMMON_H

#include <vector>

namespace Wt {

    struct ArrayInfo {
        std::vector<int> shape;
        std::vector<int> strides;
        size_t ndim;
    };

    enum Coefficient {
        COEF_APPROX = 0,
        COEF_DETAIL = 1,
    };

    enum DiscreteTransformType {
        DWT_TRANSFORM = 0,
        SWT_TRANSFORM = 1,
    };
    
    enum Mode {
       MODE_INVALID = -1,
       MODE_ZEROPAD = 0,   /* default, signal extended with zeros */
       MODE_SYMMETRIC,     /* signal extended symmetrically (mirror)
                            * also known as half-sample symmetric
                            * For extensions greater than signal length,
                            * mirror back and forth:
                            * 2 3 3 2 1 | 1 2 3 | 3 2 1 1 2
                            */
       MODE_CONSTANT_EDGE, /* signal extended with the border value */
       MODE_SMOOTH,        /* linear extrapolation (first derivative) */
       MODE_PERIODIC,      /* signal is treated as being periodic */
       MODE_PERIODIZATION, /* signal is treated as being periodic, minimal output length */
       MODE_REFLECT,       /* signal extended symmetrically (reflect)
                            * also known as whole-sample symmetric
                            * For extensions greater than signal length,
                            * reflect back and forth without repeating edge values:
                            * 1 2 3 2 | 1 2 3 | 2 1 2 3
                            */
       MODE_ANTISYMMETRIC,  /* antisymmetric version of "MODE_SYMMETRIC"
                             * also known as half-sample antisymmetric
                             * 2 3 -3 -2 -1 | 1 2 3 | -3 -2 -1 1 2
                             */
       MODE_ANTIREFLECT,    /* antisymmetric version of "MODE_REFLECT"
                             * also known as whole-sample antisymmetric
                             * 0 -1 -2 -1 0 | 1 2 3 | 4 5 6 5 4
                             */
       MODE_MAX,
    };

    /* ##### Calculating buffer lengths for various operations ##### */

    /*
     * Length of DWT coeffs for specified input data length, filter length and
     * signal extension mode.
     */
    size_t dwt_buffer_length(size_t input_len, size_t filter_len, Mode mode);

    /*
     * Length of reconstructed signal for specified input coeffs length and filter
     * length. It is used for direct reconstruction from coefficients (normal
     * convolution of upsampled coeffs with filter).
     */
    size_t reconstruction_buffer_length(size_t coeffs_len, size_t filter_len);

    /*
     * Length of IDWT reconstructed signal for specified input coeffs length, filter
     * length and extension mode.
     */
    size_t idwt_buffer_length(size_t coeffs_len, size_t filter_len, Mode mode);

    /* Length of SWT coefficients for specified input signal length (== input_len) */
    size_t swt_buffer_length(size_t input_len);

    /* Maximum useful level of DWT decomposition. */
    int dwt_max_level(size_t input_len, size_t filter_len);

    /* Maximum useful level of SWT decomposition. */
    int swt_max_level(size_t input_len);

}

#endif // WAVELET_COMMON_H

