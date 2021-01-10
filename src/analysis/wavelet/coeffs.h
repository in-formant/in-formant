#ifndef WAVELET_COEFFS_H
#define WAVELET_COEFFS_H

#define CAT_HELPER(A, B) A##B
#define CAT(A, B) CAT_HELPER(A, B)

#define NELEMS(x) (sizeof(x) / sizeof(*x))

#define TYPE double

extern const TYPE CAT(dmey_, TYPE)[62];
extern const TYPE * const CAT(db_, TYPE)[38];
extern const TYPE * const CAT(sym_, TYPE)[19];
extern const TYPE * const CAT(coif_, TYPE)[17];
extern const TYPE * const (* const CAT(bior_, TYPE)[6]);

#endif // WAVELET_COEFFS_H
