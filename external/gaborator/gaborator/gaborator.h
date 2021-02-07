//
// Constant Q spectrum analysis and resynthesis
//
// Copyright (C) 2015-2020 Andreas Gustafsson.  This file is part of
// the Gaborator library source distribution.  See the file LICENSE at
// the top level of the distribution for license information.
//

#ifndef _GABORATOR_GABORATOR_H
#define _GABORATOR_GABORATOR_H

#define __STDC_LIMIT_MACROS

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <complex>
#include <limits>
#include <map>
#include <typeinfo>

#include "gaborator/fft.h"
#include "gaborator/gaussian.h"
#include "gaborator/linear_transform.h"
#include "gaborator/pod_vector.h"
#include "gaborator/pool.h"
#include "gaborator/ref.h"
#include "gaborator/vector_math.h"


namespace gaborator {

using std::complex;

// An integer identifying an audio sample
typedef int64_t sample_index_t;

// An integer identifying a coefficient
typedef int64_t coef_index_t;

// An integer identifying a slice
typedef int64_t slice_index_t;

// See https://tauday.com/tau-manifesto
static const double tau = 2.0 * M_PI;

// Round up to next higher or equal power of 2

inline int
next_power_of_two(int x) {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

// Determine if x is a power of two.
// Note that this considers 0 to be a power of two.

static inline bool
is_power_of_two(unsigned int x) {
    return (x & (x - 1)) == 0;
}

// Given a power of two v, determine log2(v)
// https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2

static inline unsigned int whichp2(unsigned int v) {
    assert(is_power_of_two(v));
    unsigned int r = (v & 0xAAAAAAAA) != 0;
    r |= ((v & 0xCCCCCCCC) != 0) << 1;
    r |= ((v & 0xF0F0F0F0) != 0) << 2;
    r |= ((v & 0xFF00FF00) != 0) << 3;
    r |= ((v & 0xFFFF0000) != 0) << 4;
    return r;
}

// Floor division: return the integer part of a / b
// rounded down (not towards zero).  For positive b only.

inline int64_t floor_div(int64_t a, int64_t b) {
    assert(b > 0);
    if (a >= 0)
        return a / b;
    else
        return (a - b + 1) / b;
}

// Floating point modulus, the remainder r of a / b
// satisfying 0 <= r < b even for negative a.
// For positive b only.

static inline double
sane_fmod(double a, double b) {
    assert(b > 0);
    double m = fmod(a, b);
    if (m < 0)
        m += b;
    return m;
}

// Do an arithmetic left shift of a 64-bit signed integer.  This is
// what a << b ought to do, but according to the C++11 draft (n3337),
// section 5.8, that invokes undefined behavior when a is negative.
// GCC is actually smart enough to optimize this into a single shlq
// instruction.
//
// No corresponding kludge is needed for right shifts, because a right
// shift of a negative signed integer is implementation-defined, not
// undefined, and we trust implementations to define it sanely.

static inline int64_t
shift_left(int64_t a, unsigned int b) {
    if (a < 0)
        return -(((uint64_t) -a) << b);
    else
        return (((uint64_t) a) << b);
}

// Convert between complex types

template<class T, class U>
complex<T> c2c(complex<U> c) { return complex<T>(c.real(), c.imag()); }

// Convert a sequence of complex values to real

template <class I, class O>
O complex2real(I b, I e, O o) {
    while (b != e) {
        *o++ = (*b++).real();
    }
    return o;
}

// A vector-like object that allows arbitrary integer indices
// (positive or negative, but excluding the largest possible integer)
// and automatically resizes the storage.  Uses storage proportional
// to the difference between the smallest and largest index value (for
// example, if indices range from -102 to -100 (inclusive), memory use
// is on the order of 3 elements).
//
// T is the element type
// I is the integer index type

template <class T, class I = int>
struct range_vector {
    range_vector() {
        init_bounds();
    }
    range_vector(const range_vector &) = default;
    range_vector &operator=(const range_vector &rhs) = default;
    range_vector(range_vector &&rhs):
        v(std::move(rhs.v)),
        lower(rhs.lower),
        upper(rhs.upper)
    {
        rhs.init_bounds();
    }
    range_vector &operator=(range_vector &&rhs) {
        if (this == &rhs)
            return *this;
        v = std::move(rhs.v);
        lower = rhs.lower;
        upper = rhs.upper;
        rhs.init_bounds();
        return *this;
    }
private:
    void init_bounds() {
        lower = std::numeric_limits<I>::max();
        upper = std::numeric_limits<I>::min();
    }
    T *unchecked_get(I i) {
        return &v[(size_t)(i & ((I)v.size() - 1))];
    }
    const T *unchecked_get(I i) const {
        return &v[i & ((I)v.size() - 1)];
    }

public:
    // Note: Pointer returned becomes invalid when range_vector
    // is changed
    T *
    get(I i, bool create) {
        if (! has_index(i)) {
            if (create)
                extend(i);
            else
                return 0;
        }
        return unchecked_get(i);
    }

    // Get a pointer to an existing element, or null if out of range
    const T *
    get(I i) const {
        if (! has_index(i))
            return 0;
        return unchecked_get(i);
    }

    // Note: Reference returned becomes invalid when range_vector
    // is changed
    T &
    get_or_create(I i) {
        return *get(i, true);
    }

    // Get a reference to the element at index i, which must be valid
    T &
    get_existing(I i) {
        assert(has_index(i));
        return *unchecked_get(i);
    }

    // Const version of the above
    const T &
    get_existing(I i) const {
        assert(has_index(i));
        return *unchecked_get(i);
    }

private:
    void extend(I i) {
        I new_lower = lower;
        I new_upper = upper;
        if (i < lower)
            new_lower = i;
        if (i + 1 > upper)
            new_upper = i + 1;
        I old_size = v.size();
        I new_need = new_upper - new_lower;
        if (new_need > old_size) {
            if (old_size == 0) {
                v.resize(1);
            } else {
                I new_size = old_size;
                while (new_size < new_need)
                    new_size *= 2;
                v.resize(new_size);
                if (old_size) {
                    for (I j = lower; j < upper; j++) {
                        I jo = j & (old_size - 1);
                        I jn = j & (new_size - 1);
                        if (jo != jn)
                            std::swap(v[jo], v[jn]);
                    }
                }
            }
        }
        lower = new_lower;
        upper = new_upper;
    }

public:
    // Erase the elements whose index is less than "limit"
    void erase_before(I limit) {
        I i = lower;
        for (; i < upper && i < limit; i++)
            *unchecked_get(i) = T();
        lower = i;
    }

    void clear() {
        v.clear();
        init_bounds();
    }

    I begin_index() const { return lower; }
    I end_index() const { return upper; }
    bool empty() const { return lower >= upper; }
    bool has_index(I i) const { return i >= lower && i < upper; }

private:
    std::vector<T> v;
    I lower, upper;
};

// Calculate the size of the alias-free part (the "filet")
// of a signal slice of size "fftsize"

static inline unsigned int filet_part(unsigned int fftsize) {
    return fftsize >> 1;
}

// Calculate the size of the padding (the "fat") at each
// end of a signal slice of size "fftsize"

static inline unsigned fat_part(unsigned int fftsize) {
    return fftsize >> 2;
}

// Frequency band parameters shared between octaves

template<class T>
struct band_params: public refcounted {
    typedef complex<T> C;
    bool dc; // True iff this is the DC band
    unsigned int sftsize; // Size of "short FFT" spanning the band
    unsigned int sftsize_log2; // log2(sftsize)
    unsigned int step; // Signal samples per coefficient sample
    unsigned int step_log2; // log2(step)
    fft<C *> *sft; // Fourier transform for windows, of size sftsize
    std::vector<T> kernel; // Frequency-domain filter kernel
    std::vector<T> dual_kernel; // Dual of the above
    pod_vector<C> shift_kernel; // Complex exponential for fractional frequency compensation
    pod_vector<C> shift_kernel_conj; // Conjugate of the above
    int fq_offset_int; // Frequency offset in bins (big-fft bin of left window edge)
    double ff; // Center (bp) or corner (lp) frequency in units of the sampling frequency
    double center; // Center frequency in units of FFT bins
    int icenter; // Center frequency rounded to nearest integer FFT bin
    double ffsd; // Standard deviation of the bandpass Gaussian, as fractional frequency
    unsigned int time_support; // Filter support in time domain, in octave subsamples
};

// Downsampling parameters.  These have some similarity to band
// parameters, but only some.  For example, these may use a real
// rather than complex FFT for the "short FFT".

template <class T>
struct downsampling_params {
    typedef complex<T> C;
    unsigned int sftsize;
    std::vector<T> kernel; // Frequency-domain filter kernel
    std::vector<T> dual_kernel;
#if GABORATOR_USE_REAL_FFT
    rfft<C *> *rsft;
#else
    fft<C *> *sft;
#endif
};

// Forward declarations
template<class T> struct analyzer;
template<class T> struct zone;
template<class T, class C = complex<T> > struct coefs;
template<class C> struct sliced_coefs;
template <class T, class OI = complex<T> *, class C = complex<T> >
  struct row_source;
template <class T, class II = complex<T> *, class C = complex<T> >
  struct row_dest;
template <class T, class II = complex<T> *, class C = complex<T> >
  struct row_add_dest;

// Abstract class for tracking changes to a coefficient set.
// This may be used for updating a resolution pyramid of
// magnitude data.

template<class T>
struct shadow {
    virtual ~shadow() { }
    virtual void update(const coefs<complex<T> > &msc,
                        sample_index_t i0, sample_index_t i1) = 0;
    // Deprecated
    virtual void update(int oct, int ze, const sliced_coefs<complex<T> > &sc,
                        slice_index_t sli0, slice_index_t sli1) = 0;
};

// Per-band coefficient metadata, shared between octaves.

struct band_coefs_meta {
    unsigned int slice_len; // Number of coefficients per slice
    unsigned short slice_len_log2; // Log2 of the above
    // Log2 of the downsampling factor of the coefficients in
    // this band relative to the signal samples.  The value
    // applies as such in the top octave; in other octaves,
    // the octave number needs to be added.
    unsigned short step_log2;
    // Offset of the beginning of this band in the data array
    unsigned int band_offset;
};

// Per-zone coefficient metadata.  This contains information
// describing an "oct_coefs" structure that is common to many
// instances and should not be duplicated in each one.

struct zone_coefs_meta {
    typedef std::vector<band_coefs_meta> band_vector;
    void init(const band_vector &bands_) {
        bands = bands_;
        unsigned int offset = 0;
        for (unsigned int i = 0; i < bands.size(); i++) {
            bands[i].band_offset = offset;
            offset += bands[i].slice_len;
        }
        total_size = offset;
    }
    band_vector bands;
    // Total size of data array (in elements, not bytes)
    unsigned int total_size;
};

// Per-octave coefficient metadata.
// Cf. struct octave

struct oct_coefs_meta {
    zone_coefs_meta *z;
    unsigned int n_bands_above; // Total number of bands in higher octaves
};

// Coefficient metadata for multirate coefficients.

struct coefs_meta: public refcounted {
    coefs_meta() = default;
    coefs_meta(const coefs_meta &) = delete;
    unsigned int n_octaves;
    unsigned int n_bands_total;
    unsigned int bands_per_octave;
    unsigned int slice_len; // octave subsamples per slice
    std::vector<zone_coefs_meta> zones;
    std::vector<oct_coefs_meta> octaves;
};

// Split a "global band number" gbno into an octave and band
// number within octave ("obno").
//
// Global band numbers start at 0 for the band at or close to
// fs/2, and increase towards lower frequencies.
//
// Include the DC band if "dc" is true.
// Returns true iff gbno is valid.

static inline bool
bno_split(const coefs_meta &meta, int gbno, int &oct, unsigned int &obno, bool dc) {
    if (gbno < 0) {
        // Above top octave
        return false;
    } else if (gbno >= (int)meta.n_bands_total - 1) {
        // At or below DC
        if (gbno == (int)meta.n_bands_total - 1) {
            // At DC
            if (dc) {
                oct = meta.n_octaves - 1;
                obno = 0;
                return true;
            } else {
                return false;
            }
        } else {
            // Below DC
            return false;
        }
    } else {
        // Within bandpass region
        // Start by determining the octave
        int oct_tmp = gbno / meta.bands_per_octave;
        int obno_tmp = gbno % meta.bands_per_octave;
        // Octave 0 is twice the size
        if (oct_tmp < 2) {
            if (oct_tmp == 1) {
                // Octave 1 gets appended to octave 0
                obno_tmp += meta.bands_per_octave;
                oct_tmp--;
            }
        } else {
            // Because octave 0 is twice the size, the
            // other octave numbers are one less
            oct_tmp--;
        }
        oct = oct_tmp;
        // Now determine the band within the octave.
        // obno_tmp counts down, but obno counts up.
        obno = (unsigned int)meta.octaves[oct].z->bands.size() - 1 - obno_tmp;
        return true;
    }
}

// The inverse of bno_split().  Returns a gbno.  The arguments must
// be valid.

static inline
int bno_merge(const coefs_meta &meta, int oct, unsigned int obno) {
    unsigned int n_bands = (unsigned int)meta.octaves[oct].z->bands.size();
    assert(obno < n_bands);
    int bno_from_end = n_bands - 1 - obno;
    return bno_from_end + meta.octaves[oct].n_bands_above;
}

// Coefficients of a single octave for a single input signal slice.
// C is the coefficient type, typically complex<float> but can also
// be e.g. unsigned int to store cluster numbers, or float to store
// magnitudes.

template<class C>
struct oct_coefs: public refcounted {
    oct_coefs(const zone_coefs_meta &zmeta_, bool clear_ = true):
        zmeta(zmeta_),
        data(zmeta.total_size),
        bands(*this)
    {
        if (clear_)
            clear();
    }
    oct_coefs(const oct_coefs &) = delete;
    uint64_t estimate_memory_usage() const {
        return zmeta.total_size * sizeof(C) + sizeof(*this);
    }
    void clear() {
        std::fill(data.begin(), data.end(), C());
    }
    // Deep copy
    oct_coefs &operator=(const oct_coefs &rhs) {
        assert(data.size() == rhs.data.size());
        memcpy(data.data(), rhs.data.data(), data.size() * sizeof(C));
        return *this;
    }

    const zone_coefs_meta &zmeta;

    // The data for all the bands are allocated together
    // as a single vector to reduce the number of allocations
    pod_vector<C> data;
    // Vector-like collection of pointers into "data", one for each band
    struct band_array {
        band_array(oct_coefs &outer_): outer(outer_) { }
        C *operator[](size_t i) const {
            return outer.data.data() + outer.zmeta.bands[i].band_offset;
        }
        size_t size() const { return outer.zmeta.bands.size(); }
        oct_coefs &outer;
    } bands;
};

// Add the oct_coefs "b" to the oct_coefs "a"

template <class C>
void add(oct_coefs<C> &a, const oct_coefs<C> &b) {
    size_t n_bands = a.bands.size();
    assert(n_bands == b.bands.size());
    for (size_t obno = 0; obno < n_bands; obno++) {
        unsigned int len = a.zmeta.bands[obno].slice_len;
        C *band_a = a.bands[obno];
        C *band_b = b.bands[obno];
        for (unsigned int j = 0; j < len; j++) {
            band_a[j] += band_b[j];
        }
    }
}

// Sliced coefficients.  These cover an arbitrary time range, but only
// a single octave.  Template argument is as for struct oct_coefs.
// This is default constructible so that we can create an array of
// them, but not usable until "meta" has been set up.

template<class C>
struct sliced_coefs {
    typedef range_vector<ref<oct_coefs<C> >, slice_index_t> slices_t;
    uint64_t estimate_memory_usage() const {
        unsigned int n = 0;
        size_t size_each = 0;
        for (slice_index_t sl = slices.begin_index(); sl < slices.end_index(); sl++) {
            const ref<oct_coefs<C> > &t = slices.get_existing(sl);
            if (t) {
                if (! size_each)
                    size_each = (size_t)t->estimate_memory_usage();
                n++;
            }
        }
        return n * size_each;
    }
    void clear() {
        slices.clear();
    }
    zone_coefs_meta *meta;
    slices_t slices;
};

// Get a pointer to an existing existing coefficient slice,
// or null if one does not exist.  Like get_or_create_coefs(),
// this hides the distinction between the two types of nonexistence.

template <class CT>
oct_coefs<CT> *get_existing_coefs(const sliced_coefs<CT> &sc,
                                  slice_index_t i)
{
    const ref<oct_coefs<CT>> *p = sc.slices.get(i);
    if (! p)
        return 0;
    return p->get();
}

// Get an existing coefficient slice, or create a new one.  Note that
// this hides the distinction between two types of nonexistence: that
// of slices outside the range of the range_vector, and that of
// missing slices within the range (having a null ref).  CT is the
// coefficient type, which is typically C aka complex<T>, but can be
// different, for example float to represent magnitudes.

template <class CT>
oct_coefs<CT> &get_or_create_coefs(sliced_coefs<CT> &sc, slice_index_t i) {
    ref<oct_coefs<CT> > &p(sc.slices.get_or_create(i));
    if (! p)
        p.reset(new oct_coefs<CT>(*sc.meta));
    return *p;
}

// Return the signal sample time corresponding to coefficient sample 0
// of coefficient slice 0, for slices of length len.  It would be nice
// if this were zero, but for historical reasons, it's offset by half
// a slice (corresponding to the analysis fat).

static inline int coef_offset(int len) {
    return len >> 1;
}

// Get the base 2 logarithm of the downsampling factor of
// band "obno" in octave "oct"

static inline int
band_scale_exp(const zone_coefs_meta &meta, int oct, unsigned int obno) {
    return meta.bands[obno].step_log2 + oct;
}

// Return the coefficient index (the time in terms of coefficient
// subsamples) of the first cofficient of slice "sli" of band
// "obno" in octave "oct"

static inline coef_index_t
coef_time(const zone_coefs_meta &meta, slice_index_t sli, int oct, int obno) {
    int len = meta.bands[obno].slice_len;
    return coef_offset(len) + sli * len;
}

// Return the sample index (the time in terms of samples) time of
// coefficient "i" in slice "sli" of band "obno" in octave "oct"

static inline sample_index_t
sample_time(const zone_coefs_meta &meta, slice_index_t sli, int i, int oct, int obno) {
    coef_index_t sst = coef_time(meta, sli, oct, obno) + i;
    return shift_left(sst, band_scale_exp(meta, oct, obno));
}

// Multirate sliced coefficients.  These cover an arbitrary time
// range and the full frequency range (all octaves).
// Template arguments:
//     T analyzer sample data type
//     C coefficient data type
// Note default for template argument C defined in forward declaration.

template<class T, class C>
struct coefs {
    typedef C value_type;
    coefs(const analyzer<T> &anl_, shadow<T> *shadow_ = 0):
        octaves(anl_.n_octaves), shadow0(shadow_)
    {
        // Use the analysis structure of the largest plan
        meta = anl_.anl_plans[anl_.anl_plans.size() - 1]->cmeta;
        // Set up shortcut pointer to zone metadata in each octave
        for (unsigned int oct = 0; oct < octaves.size(); oct++)
            octaves[oct].meta = meta->octaves[oct].z;
    }
    uint64_t estimate_memory_usage() const {
        uint64_t s = 0;
        for (unsigned int oct = 0; oct < octaves.size(); oct++)
            s += octaves[oct].estimate_memory_usage();
        return s;
    }
    void clear() {
        for (unsigned int oct = 0; oct < octaves.size(); oct++)
            octaves[oct].clear();
    }
    ref<coefs_meta> meta;
    std::vector<sliced_coefs<C> > octaves;
    shadow<T> *shadow0;
};

// Read coefficients i0..i1 of band gbno in msc into buf.

template <class T, class C>
void read(const coefs<T, C> &msc, int gbno,
          coef_index_t i0, coef_index_t i1, C *buf)
{
    int oct;
    unsigned int obno; // Band number within octave
    bool valid = gaborator::bno_split(*msc.meta, gbno, oct, obno, true);
    assert(valid);
    row_source<T, C *, C>(msc, oct, obno)(i0, i1, buf);
}

template <class T, class C>
void write(coefs<T, C> &msc, int gbno,
           coef_index_t i0, coef_index_t i1, C *buf)
{
    int oct;
    unsigned int obno; // Band number within octave
    bool valid = gaborator::bno_split(*msc.meta, gbno, oct, obno, true);
    assert(valid);
    row_dest<T, C *, C>(msc, oct, obno)(i0, i1, buf);
}

template <class T, class C>
void add(coefs<T, C> &msc, int gbno,
         coef_index_t i0, coef_index_t i1, C *buf)
{
    int oct;
    unsigned int obno; // Band number within octave
    bool valid = gaborator::bno_split(*msc.meta, gbno, oct, obno, true);
    assert(valid);
    row_add_dest<T, C *, C>(msc, oct, obno)(i0, i1, buf);
}

// Return the base 2 logarithm of the time step (aka downsampling
// factor) of band "gbno".

static inline
unsigned int band_step_log2(const coefs_meta &meta, int gbno) {
    int oct;
    unsigned int obno;
    bool valid = bno_split(meta, gbno, oct, obno, true);
    assert(valid);
    return band_scale_exp(*meta.octaves[oct].z, oct, obno);
}


// Convert a signal time t into a coefficient sample
// index.  t must coincide with a coefficient sample time.

static inline
coef_index_t t2i_exact(const coefs_meta &meta, int gbno, sample_index_t t) {
    int shift = band_step_log2(meta, gbno);
    int64_t mask = ((sample_index_t)1 << shift) - 1;
    assert((t & mask) == 0);
    return t >> shift;
}

// Read a single coefficient sample at signal time t,
// which must coincide with a coefficient sample time

template <class T, class C>
C read1t(const coefs<T, C> &msc, int gbno, sample_index_t t) {
    coef_index_t i = t2i_exact(*msc.meta, gbno, t);
    C c;
    read(msc, gbno, i, i + 1, &c);
    return c;
}

// Read a single coefficient sample at signal time t,
// which must coincide with a coefficient sample time

template <class T, class C>
void write1t(coefs<T, C> &msc, int gbno, sample_index_t t, C c) {
    coef_index_t i = t2i_exact(*msc.meta, gbno, t);
    write(msc, gbno, i, i + 1, &c);
}

// Perform an fftshift of the range between iterators a and b.
// Not optimized - not for use in inner loops.

template <class I>
void fftshift(I b, I e) {
    size_t len = e - b;
    assert(len % 2 == 0);
    for (size_t i = 0; i < len / 2; i++)
        std::swap(*(b + i), *(b + len / 2 + i));
}

// Given an unsigned index i into an FFT of a power-of-two size
// "size", return the corresponding signed index, ranging from -size/2
// to size/2-1.  This is equivalent to sign extension of an integer of
// log2(size) bits; see Hacker's Delight, page 18.  This can be used
// to convert an FFT index into a frequency (positive or negative;
// note that Nyquist is considered negatitive = -fs/2).

static inline int signed_index(unsigned int i, unsigned int size) {
    unsigned int t = size >> 1;
    return (i ^ t) - t;
}

// Construct a frequency-domain lowpass filter whose response is the
// convolution of a rectangle and a gaussian.  The cutoff freqency is
// ff_cutoff (a fractional frequency), and the standard deviation of
// the gaussian is ff_sd.  The returned filter covers the full
// frequency range from 0 to fs (with negative frequencies at the end,
// the usual convention for FFT spectra).
//
// When center=true, construct a time-domain window instead,
// passing the center of the time-domain signal.
//
// The result is stored between iterators b and e, which must have a
// real value_type.

template <class I>
inline void gaussian_windowed_lowpass(double ff_cutoff, double ff_sd,
                                      I b, I e, bool center = false)
{
    size_t len = e - b;
    double inv_len = 1.0 / len;
    for (I it = b; it != e; ++it) {
        size_t i = it - b;
        double thisff;
        if (center)
            // Symmetric around center
            thisff = std::abs(i - (len * 0.5)) * inv_len;
        else
            // Symmetric around zero
            thisff = (i > len / 2 ? len - i : i) * inv_len;
        double x = thisff - ff_cutoff;
        double v = gaussian_edge(ff_sd, -x);
        *it = v;
    }
}

// A set of octaves having identical parameters form a "zone",
// and their shared parameters are stored in a "struct zone".

template <class T>
struct zone: public refcounted {
    zone(): n_bands(0) { }
    ~zone() { }
    // Zone number, 0..3
    unsigned int zno;
    // Total number of bands, including DC band if lowest octave
    unsigned int n_bands;
    // Band parameters by increasing frequency; DC band is index 0 if
    // present
    std::vector<ref<band_params<T> > > bandparams;
    // Pseudo-bands mimicing the response of bands in the
    // neighboring octaves, used for calculating the duals only
    std::vector<ref<band_params<T> > > mock_bandparams;
    pod_vector<T> power;
    pod_vector<T> power_nodc;
    unsigned int max_step_log2;
};

template <class T>
struct octave {
    zone<T> *z;
};

// Helper function for pushing parameters onto the vectors in struct zone

template <class T>
void push(std::vector<ref<band_params<T> > > &v, band_params<T> *p) {
    v.push_back(ref<band_params<T> >(p));
}

// Phase conventions: phase_convention::absolute means the phase of a
// coefficient at time tc is relative to e(i tau f t), and
// phase_convention::relative means it is relative to
// e(i tau f (t - tc))

enum class phase_convention { absolute, relative };

enum class bandwidth_formula { v1, v2 };

// A set of spectrum analysis parameters

struct parameters {
    parameters(unsigned int bands_per_octave_, double ff_min_,
               double ff_ref_ = 1.0,
               double overlap_ = 0.7,
               double max_error_ = 1e-5,
               phase_convention phase_ = phase_convention::absolute,
               bandwidth_formula bandwidth_ = bandwidth_formula::v1):
        bands_per_octave(bands_per_octave_),
        ff_min(ff_min_),
        ff_ref(ff_ref_),
        overlap(overlap_),
        max_error(max_error_),
        phase(phase_),
        bandwidth(bandwidth_),
        coef_scale(1.0),
        synthesis(true)
    { }
    // Provide an operator< so that we can create a set or map of parameters
    bool operator<(const parameters &b) const {
#define GABORATOR_COMPARE_LESS(member) do { \
        if (member < b.member) \
            return true; \
        if (member > b.member) \
            return false; \
        } while(0)
        GABORATOR_COMPARE_LESS(bands_per_octave);
        GABORATOR_COMPARE_LESS(ff_min);
        GABORATOR_COMPARE_LESS(ff_ref);
        GABORATOR_COMPARE_LESS(overlap);
        GABORATOR_COMPARE_LESS(max_error);
        GABORATOR_COMPARE_LESS(phase);
#undef GABORATOR_COMPARE_LESS
        // Equal
        return false;
    }
    bool operator==(const parameters &b) const {
        return !((*this < b) || (b < *this));
    }
    // The frequency increases by a factor of band_spacing from
    // one bandpass band to the next.
    double band_spacing_log2() const {
        return 1.0 / bands_per_octave;
    }
    double band_spacing() const {
        return exp2(band_spacing_log2());
    }
    // The standard deviation of the Gaussian in units of the mean
    double sd() const {
        return overlap *
            (bandwidth == bandwidth_formula::v1 ?
             band_spacing() - 1 :
             log(2) / bands_per_octave);
    }

    // Defining Q as the frequency divided by the half-power bandwidth,
    // we get
    //
    // norm_gaussian(sd, hbw) = sqrt(2)
    //
    // (%i1) e1: exp(-(hbw * hbw) / (2 * sd * sd)) = 1 / sqrt(2);
    // (%i2) solve(e1, hbw);
    // (%o2) [hbw = - sqrt(log(2)) sd, hbw = sqrt(log(2)) sd]
    double q() const {
        return 1.0 / (2 * sqrt(log(2)) * sd());
    }

    template<class T> friend class analyzer;
    unsigned int bands_per_octave;
    double ff_min;
    double ff_ref;
    double overlap;
    double max_error;
    phase_convention phase;
    bandwidth_formula bandwidth;
    double coef_scale;
    bool synthesis; // Synthesis is supported
};

// Like std::fill, but returns the end iterator

template <class I, class T>
I fill(I b, I e, T v) {
    std::fill(b, e, v);
    return e;
}

// Multiply a vector by a scalar, in-place.
// Used only at the setup stage, so performance is not critical.

template <class V, class S>
void scale_vector(V &v, S s) {
    for (size_t i = 0; i < v.size(); i++)
        v[i] *= s;
}

// Zero-padding source wrapper.  This returns data from the underlying
// source within the interval src_i0 to src_i1, and zero elsewhere.

template <class S, class T>
struct zeropad_source {
    zeropad_source(const S &source_, int64_t src_i0_, int64_t src_i1_):
        source(source_), src_i0(src_i0_), src_i1(src_i1_)
    { }
    T *operator()(int64_t i0, int64_t i1, T *output) const {
        int64_t overlap_begin = std::max(i0, src_i0);
        int64_t overlap_end = std::min(i1, src_i1);
        if (overlap_end <= overlap_begin) {
            // No overlap
            output = gaborator::fill(output, output + (i1 - i0), (T)0);
        } else {
            // Some overlap
            if (overlap_begin != i0) {
                output = gaborator::fill(output, output + (overlap_begin - i0), (T)0);
            }
            output = source(overlap_begin, overlap_end, output);
            if (overlap_end != i1) {
                output = gaborator::fill(output, output + (i1 - overlap_end), (T)0);
            }
        }
        return output;
    }
    const S &source;
    int64_t src_i0, src_i1;
};

template <class T>
struct pointer_source {
    pointer_source(const T *p_, int64_t buf_i0_, int64_t buf_i1_):
        p(p_), buf_i0(buf_i0_), buf_i1(buf_i1_) { }
    T *operator()(int64_t i0, int64_t i1, T *output) const {
        assert(i1 >= i0);
        assert(i0 >= buf_i0);
        assert(i1 <= buf_i1);
        return std::copy(p + (i0 - buf_i0), p + (i1 - buf_i0), output);
    }
    const T *p;
    int64_t buf_i0, buf_i1;
};

// Fill the buffer at dst, of length dstlen, with data from src where
// available, otherwise with zeroes.  The data in src covers dst indices
// from i0 (inclusive) to i1 (exclusive).

template <class T>
void copy_overlapping_zerofill(T *dst, size_t dstlen, const T *src,
                               int64_t i0, int64_t i1)
{
    pointer_source<T> ps(src, i0, i1);
    zeropad_source<pointer_source<T>, T> zs(ps, i0, i1);
    zs(0, dstlen, dst);
}

// Given a set of FFT coefficients "coefs" of a real
// sequence, where only positive-frequency coefficients
// (including DC and Nyquist) are valid, return the
// coefficient for an arbitrary frequency index "i"
// which may correspond to a negative frequency, or
// even an alias outside the range (0..fftsize-1).

template<class T>
complex<T> get_real_spectrum_coef(complex<T> *coefs, int i, unsigned int fftsize) {
    i &= fftsize - 1;
    // Note that this is >, not >=, becase fs/2 is considered nonnegative
    bool neg_fq = (i > (int)(fftsize >> 1));
    if (neg_fq) {
        i = fftsize - i;
    }
    complex<T> c = coefs[i];
    if (neg_fq) {
        c = conj(c);
    }
    return c;
}

// A set of buffers of various sizes used for temporary vectors during
// analysis.  These are allocated as a single block to reduce the
// number of dynamic memory allocations.

template <class T>
struct buffers {
    static const size_t maxbufs = 10;
    typedef complex<T> C;
    buffers(unsigned int fftsize_max,
            unsigned int sftsize_max):
        n(0)
    {
        offset[0] = 0;
        // Define the size of each buffer
        def(fftsize_max * sizeof(C)); // 0
        def(fftsize_max * sizeof(C)); // 1
        def(sftsize_max * sizeof(C)); // 2
        def(sftsize_max * sizeof(C)); // 3
        def(sftsize_max * sizeof(C)); // 4
        def(fftsize_max * sizeof(T)); // 5
        assert(n <= maxbufs);
        data = ::operator new(offset[n]);
    }
    ~buffers() {
        ::operator delete(data);
    }
    void def(size_t size) {
        size_t o = offset[n++];
        offset[n] = o + size;
    }
    // A single buffer of element type E
    template<class E>
    struct buffer {
        typedef E *iterator;
        buffer(void *b_, void *e_):
            b((E *)b_), e((E *)e_)
        { }
        iterator begin() const { return b; }
        iterator end() const { return e; }
        E *data() { return b; }
        const E *data() const { return b; }
        E &operator[](size_t i) { return b[i]; }
        const E &operator[](size_t i) const { return b[i]; }
        size_t size() const { return e - b; }
    private:
        E *b;
        E *e;
    };
    // Get buffer number "i" as a vector-like object with element type "E"
    // and a length of "len" elements.
    template <class E>
    buffer<E> get(size_t i, size_t len) {
        len *= sizeof(E);
        size_t o = offset[i];
        assert(len <= offset[i + 1] - o);
        return buffer<E>((char *)data + o, (char *)data + o + len);
    }
private:
    void *data;
    size_t n;
    size_t offset[maxbufs + 1];
};

// Get the bounds of the range of existing coefficients for a
// given band, in units of coefficient samples.

template <class T, class C>
void get_band_coef_bounds(const coefs<T, C> &msc, int oct, unsigned int obno,
                          coef_index_t &ci0_ret, coef_index_t &ci1_ret)
{
    const sliced_coefs<C> &sc = msc.octaves[oct];
    const typename sliced_coefs<C>::slices_t &slices = sc.slices;
    // Convert from slices to coefficient samples
    ci0_ret = coef_time(*sc.meta, slices.begin_index(), oct, obno);
    ci1_ret = coef_time(*sc.meta, slices.end_index(), oct, obno);
}

template <class T, class C>
void get_band_coef_bounds(const coefs<T, C> &msc, int gbno,
                          coef_index_t &ci0_ret, coef_index_t &ci1_ret)
{
    int oct;
    unsigned int obno; // Band number within octave
    bool r = gaborator::bno_split(*msc.meta, gbno, oct, obno, true);
    assert(r);
    get_band_coef_bounds(msc, oct, obno, ci0_ret, ci1_ret);
}

template<class T>
struct analyzer: public refcounted {
    typedef complex<T> C;
    struct plan;

    analyzer(const parameters &params_):
        params(params_),
        fftsize_max(0),
        sftsize_max(0)
    {
        // Sanity check
        assert(params.ff_min < 0.5);

        band_spacing_log2 = params.band_spacing_log2();
        band_spacing = params.band_spacing();

        // The tuning adjustment, as a log2ff.  This is a number between
        // 0 and band_spacing_log2, corresponding to a frequency at
        // or slightly above the sampling frequency where a band
        // center would fall if they actually went that high.
        // Tuning is done by increasing the center frequencies of
        // all bands by this amount relative to the untuned case
        // where one band would fall on fs exactly.
        tuning_log2ff = sane_fmod(log2(params.ff_ref), band_spacing_log2);

        // Calculate the frequency of band0, the "fs/8 band", the
        // lowest band in each octave except possibly the lowest octave.
        // Its frequency will fs/8, or a fraction of the band spacing higher
        // due to tuning. The magic -3 derives from fs/8 as log2(1/8).
        band0_log2ff = -3 + tuning_log2ff;

        // Calculate the total number of bands needed so that
        // the lowest band has a frequency <= params.ff_min.
        // end_log2ff = the log2ff of the band after the last (just past fs/2);
        // the -1 below is the log of the /2 above
        double end_log2ff = tuning_log2ff - 1;
        n_bandpass_bands_total =
            (unsigned int)ceil((end_log2ff - log2(params.ff_min)) /
                               band_spacing_log2);
        n_bands_total = n_bandpass_bands_total + 1;

        top_band_log2ff = end_log2ff - band_spacing_log2;

        ffref_gbno = (int)rint((top_band_log2ff - log2(params.ff_ref)) /
                               band_spacing_log2);

        // Establish linar transforms for converting between
        // log-frequencies (log2(ff)) and bandpass band numbers.
        // Derivation:
        //ff = exp2(tuning_log2ff - 1 - (gbno + 1) * band_spacing_log2)
        //log2(ff) = tuning_log2ff - 1 - (gbno + 1) * band_spacing_log2
        //tuning_log2ff - 1 - (gbno + 1) * band_spacing_log2 = log2(ff)
        //-(gbno + 1) * band_spacing_log2 = log2(ff) - tuning_log2ff + 1
        //-(gbno + 1) * band_spacing_log2 = log2(ff) - tuning_log2ff + 1
        //-(gbno + 1) = (log2(ff) - tuning_log2ff + 1) / band_spacing_log2
        //-gbno - 1 = (log2(ff) - tuning_log2ff + 1) / band_spacing_log2
        //-gbno = ((log2(ff) - tuning_log2ff + 1) / band_spacing_log2) + 1
        //gbno = -(((log2(ff) - tuning_log2ff + 1) / band_spacing_log2) + 1)
        //gbno = a log2(ff) + b,
        //       where a = -1 / band_spacing_log2 = -params.bands_per_octave
        //       and b = -a * tuning_log2ff + a - 1
        // The cast to double is necessary because we can't take the
        // negative of an unsigned int.
        double a = -(double)params.bands_per_octave;
        double b = -a * tuning_log2ff + a - 1;
        log2ff_bandpass_band = linear_transform(a, b);
        bandpass_band_log2ff = log2ff_bandpass_band.inverse();

        // Calculate the kernel support needed for band0,
        // and size the FFT accordingly.  This duplicates some
        // code at the beginning of make_band().
        double band0_ff = exp2(band0_log2ff);
        double band0_time_sd = time_sd(band0_ff);

        // Figure out the number of octaves, keeping in mind that
        // the top octave is twice the size, and DC band is added
        // to the bottom octave even if that makes it larger than
        // the others.
        n_octaves = (n_bandpass_bands_total + params.bands_per_octave - 1) /
            params.bands_per_octave;
        if (n_octaves > 1)
            n_octaves--;

        {
            // Precalculate the parameters of the downsampling filter.
            // These are the same for all plans, and need to be
            // calculated before creating the plans; in particular, we
            // need to know the support before we can create the
            // plans, because in low-bpo cases, it can determine the
            // minimum amount of fat needed.  The actual filter
            // coefficients are plan-dependent and will be calculated
            // later.

            // In terms of the post-downsampling sampling frequency,
            // the downsampling lowpass filter transition band starts
            // at the top edge of the highest band, and ranges to
            // fs/2.  We use the worst-case center frequency for the
            // highest band, fs/4, and add the support.
            double f0 = 0.25 + gaussian_support(ff_sd(0.25), params.max_error);
            double f1 = 0.5;

            // The cutoff frequency is in the center of the transition band
            double ff = (f0 + f1) * 0.5;
            double support = (f1 - f0) * 0.5;
            double ff_sd = gaussian_support_inv(support, params.max_error);

            // Calculate and save the time-domain support of the
            // downsampling lowpass filter for use in analyze_sliced().
            double time_sd = sd_f2t(ff_sd);

            // Set members
            ds_ff = ff;
            ds_ff_sd = ff_sd;
            ds_time_support = gaussian_support(time_sd, params.max_error);
        }

        band0_time_analysis_support =
            gaussian_support(band0_time_sd, params.max_error);
        band0_time_synthesis_support =
            band0_time_analysis_support * synthesis_support_multiplier();

        // Make analysis plans
        unsigned int max_support =
            std::max(ceil(band0_time_analysis_support),
                     ds_time_support * 2);

        unsigned int size = next_power_of_two(max_support * 2);
        plan *p;
        for (;;) {
            p = new plan(this, false, size, max_support);
            if (p->ok)
                break;
            delete p;
            size *= 2;
        }
        anl_plans.push_back(p); // Smallest possible plan
        size *= 2;
        p = new plan(this, false, size, max_support);
        assert(p->ok);
        anl_plans.push_back(p); // Next larger plan

        if (params.synthesis) {
            // Make synthesis plan (only one for now)
            max_support = std::max(ceil(band0_time_synthesis_support),
                                   ds_time_support * 2);
            // Room for at at least the two fats + as much filet
            size = next_power_of_two(max_support * 2) * 2;
            p = new plan(this, true, size, max_support);
            assert(p->ok);
            syn_plans.push_back(p);
        }

        // Find the largest fftsize and sftsize of any plan
        for (size_t i = 0; i < anl_plans.size(); i++) {
            fftsize_max = std::max(fftsize_max, anl_plans[i]->fftsize);
            sftsize_max = std::max(sftsize_max, anl_plans[i]->sftsize_max);
        }
        for (size_t i = 0; i < syn_plans.size(); i++) {
            fftsize_max = std::max(fftsize_max, syn_plans[i]->fftsize);
            sftsize_max = std::max(sftsize_max, syn_plans[i]->sftsize_max);
        }

        cmeta_any = anl_plans[0]->cmeta;
    }

    // Given a fractional frequency, return the standard deviation
    // of the frequency-domain window as a fractional frequency
    double ff_sd(double ff) const { return params.sd() * ff; }

    // Given a fractional frequency, return the standard deviation
    // of the time-domain window in samples.
    //
    // ff_sd = 1.0 / (tau * t_sd)
    // per http://users.ece.gatech.edu/mrichard/Gaussian%20FT%20and%20random%20process.pdf
    // and python test program gaussian-overlap.py
    // => (tau * t_sd) * ff_sd = 1.0
    // => t_sd = 1.0 / (tau * f_sd)
    double time_sd(double ff) const { return 1.0 / (tau * ff_sd(ff)); }

    double q() const { return params.q(); }

    // Find the worst-case time support of the analysis filters, i.e.,
    // the largest distance in time between a signal sample and a
    // coefficient affected by that sample.
    double analysis_support() const {
        return analysis_support(n_bands_total - 2); // Last before DC
    }

    // Find the time support of the analysis filter for bandpass band
    // number gbno
    double analysis_support(double gbno) const {
        return gaussian_support(time_sd(bandpass_band_ff(gbno)), params.max_error);
    }

    // Ditto for the resynthesis filters.
    double synthesis_support() const {
        return analysis_support() * synthesis_support_multiplier();
    }

    double synthesis_support(double gbno) const {
        return analysis_support(gbno) * synthesis_support_multiplier();
    }

    // The synthesis support multiplier, the factor by which the
    // synthesis filters are wider than the analysis filters in
    // the time domain.
    double synthesis_support_multiplier() const {
        // Empirical formula
        return params.synthesis ? 1.8 / params.overlap : 1;
    }

    // Return the fractional frequency of relative band number
    // "rbno" (relative to the sampling frequency of its octave).
    double rbno_ff(double rbno) const {
        double log2ff = band0_log2ff + rbno * band_spacing_log2;
        return exp2(log2ff);
    }

    // Get the center frequency of bandpass band "gbno", which
    // need not be a valid bandpass band number; out-of-range
    // arguments will return extrapolated frequencies based on
    // the logarithmic band spacing.
    double bandpass_band_ff(double gbno) const {
        return exp2(bandpass_band_log2ff(gbno));
    }

    // Get the band number of the bandpass band corresponding
    // to the fractional frequency "ff", as a floating point
    // number.  This is the inverse of bandpass_band_ff().
    double ff_bandpass_band(double ff) const {
        return log2ff_bandpass_band(log2(ff));
    }

    // Return the fractional frequency of bandpass band number "obno"
    // in octave "oct", scaling according to the octave
    double bandpass_band_ff(int oct, int obno) const {
        return bandpass_band_ff(bno_merge(oct, obno));
    }

    const plan &choose_plan(const std::vector<ref<plan>> &plans,
                            int64_t size) const
    {
        unsigned int i = 0;
        while (i < plans.size() - 1 && plans[i]->filet_size < size)
            i++;
        return *plans[i];
    }

    void
    synthesize_one_slice(int oct, const plan &plan, const coefs<T> &msc,
                         const pod_vector<T> &downsampled,
                         sample_index_t t0,
                         T *signal_out,
                         pod_vector<C> &buf0, // fftsize
                         pod_vector<C> &buf2, // largest sftsize
                         pod_vector<C> &buf3  // largest sftsize
                         ) const
    {
        zone<T> &z = *plan.octaves[oct].z;
        pod_vector<C> &signal(buf0);
        std::fill(signal.begin(), signal.end(), (T)0);

        pod_vector<C> &coefbuf(buf3);

        for (unsigned int obno = 0; obno < z.bandparams.size(); obno++) {
            band_params<T> *bp = z.bandparams[obno].get();

            // log2 of the coefficient downsampling factor
            int coef_shift = bp->step_log2;
            coef_index_t ii = t0 >> coef_shift;

            read(msc, bno_merge(oct, obno), ii, ii + bp->sftsize, coefbuf.data());
            C *indata = coefbuf.data();

            pod_vector<C> &sdata(buf2);

            T scale_factor = (T)1 / ((T)params.coef_scale * bp->sftsize);

            // Apply phase correction, adjust for non-integer center
            // frequency, and apply scale factor.  Note that phase
            // must be calculated in double precision.
            double ff = bp->center * plan.inv_fftsize_double;
            double arg = (params.phase == phase_convention::absolute) ?
                tau * t0 * ff : 0;
            C phase_times_scale = C(cos(arg), sin(arg)) * scale_factor;
            elementwise_product_times_scalar(sdata.data(), indata,
                                             bp->shift_kernel_conj.data(),
                                             phase_times_scale, bp->sftsize);

            // Switch to frequency domain
            bp->sft->transform(sdata.data());

            // Multiply signal spectrum by frequency-domain dual window,
            // accumulating result in signal.

            for (unsigned int i = 0; i < bp->sftsize; i++) {
                int iii = (bp->fq_offset_int + i) & (plan.fftsize - 1);
                // Note the ifftshift of the input index, as f=0
                // appears in the middle of the window
                C v = sdata[i ^ (bp->sftsize >> 1)] * bp->dual_kernel[i];
                // Frequency symmetry
                signal[iii] += v;
                if (! bp->dc)
                    signal[(plan.fftsize - iii) & (plan.fftsize - 1)] += conj(v);
            }
        }

        if (oct + 1 < (int) n_octaves) {
            // Upsample the downsampled data from the lower octaves
            pod_vector<C> &sdata(buf2);
            assert(downsampled.size() == plan.dsparams.sftsize);
            assert(sdata.size() >= plan.dsparams.sftsize);
#if GABORATOR_USE_REAL_FFT
            plan.dsparams.rsft->transform(downsampled.data(), sdata.begin());
#else
            // Real to complex
            std::copy(downsampled.begin(), downsampled.end(), sdata.begin());
            plan.dsparams.sft->transform(sdata.data());
#endif
            for (unsigned int i = 0; i < plan.dsparams.sftsize; i++) {
                sdata[i] *= plan.dsparams.dual_kernel
                    [i ^ (plan.dsparams.sftsize >> 1)];
            }

            // This implicitly zero pads the spectrum, by
            // not adding anything to the middle part
            // The splitting of the nyquist band is per
            // http://dsp.stackexchange.com/questions/14919/upsample-data-using-ffts-how-is-this-exactly-done
            // but should not really matter because
            // there should be no energy there to speak of
            // thanks to the windowing above.
            assert(plan.dsparams.sftsize == plan.fftsize / 2);
            unsigned int i;
            for (i = 0; i < plan.dsparams.sftsize / 2; i++)
                signal[i] += sdata[i];
            //C nyquist = sdata[i] * (T)0.5;
            C nyquist = sdata[i] * (T)0.5;
            signal[i] += nyquist;
            signal[i + plan.fftsize / 2] += nyquist;
            i++;
            for (;i < plan.dsparams.sftsize; i++)
                signal[i + plan.fftsize / 2] += sdata[i];
        }

        // Switch to time domain
#if GABORATOR_USE_REAL_FFT
        plan.rft->itransform(signal.data(), signal_out);
#else
        plan.ft->itransform(signal.data());
        // Copy real part to output
        complex2real(signal.begin(), signal.end(), signal_out);
#endif
    }

private:

    // Analyze a signal segment consisting of any number of samples.
    // oct is the octave; this is 0 except in recursive calls
    // real_signal points to the first sample
    // t0 is the sample time of the first sample
    // t1 is the sample time of the sample after the last sample
    // coefficients are added to msc

    void
    analyze_sliced(buffers<T> &buf, int oct, const T *real_signal,
                   sample_index_t t0, sample_index_t t1,
                   coefs<T> &msc) const
    {
        const plan &plan(choose_plan(anl_plans, t1 - t0));

        // Even though we don't align the FFTs to full filet-size
        // slices in this code path, we still need to align them to
        // coefficient samples so that we don't have to do expensive
        // sub-sample time shifts.  Specifically, we need to align
        // them to the largest coefficient time step of the octave.
        // slice_t0 is the sample time of the first sample in the
        // filet (not the FFT as a whole).
        zone<T> &z = *plan.octaves[oct].z;
        sample_index_t slice_t0 = t0 & ~((1 << z.max_step_log2) - 1);
        //printf("slice t0 = %d\n", (int)slice_t0);

        // Find the number of slices we need to divide the signal into.
        // We need it ahead of time so that we can size the "downsampled"
        // array accordingly.
        uint64_t n_slices = ((t1 - slice_t0) + (plan.filet_size - 1)) / plan.filet_size;

        // The "downsampled" array needs to fit one filet per slice +
        // the fat on each side, all of half size thanks to downsampling.
        // Length of each downsampled slice (including padding)
        uint64_t dstotlen = ((n_slices * plan.filet_size) + (2 * plan.fat_size)) >> 1;

        // The range of sample times covered by the "downsampled" array
        sample_index_t tmp = slice_t0 - (int)plan.fat_size;
        assert((tmp & 1) == 0);
        sample_index_t dst0 = tmp >> 1;
        sample_index_t dst1 = dst0 + (int64_t)dstotlen;

        // Not all of the "downsampled" array actually contains
        // nonzero data.  Calculate adjusted bounds to use in the
        // recursive analysis so that we don't needlessly analyze
        // zeroes.
        int ds_support = ds_time_support;
        sample_index_t dst0a = std::max(dst0, (t0 >> 1) - ds_support);
        sample_index_t dst1a = std::min(dst1, (t1 >> 1) + 1 + ds_support);

        // Buffer for the downsampled signal.  Since the size depends
        // on the total amount of signal analyzed in this call (being
        // about half of it), it can't be preallocated, but has to be
        // dynamically allocated in each call.
        pod_vector<T> downsampled(dstotlen);
        // "downsampled" will be added to, not assigned to, so we need
        // to set it to zero initially.
        std::fill(downsampled.begin(), downsampled.end(), 0);

        auto slice(buf.template get<T>(5, plan.fftsize));
        // Clear the fat on both ends (once)
        std::fill(slice.data(), slice.data() + plan.fat_size, 0);
        std::fill(slice.data() + slice.size() - plan.fat_size,
                  slice.data() + slice.size(), 0);

        // For each slice. Note that slice_i counts from 0, not from
        // the slice index of the first slice.
        for (uint64_t slice_i = 0; slice_i < n_slices; slice_i++) {
            if (slice_t0 >= t1)
                break;
            sample_index_t slice_t1 = std::min(slice_t0 + plan.filet_size, t1);
            // Copy into filet part of aligned buffer, possibly zero padding
            // if the remaining signal is shorter than a full slice.
            copy_overlapping_zerofill(slice.data() + (int)plan.fat_size,
                                      plan.filet_size,
                                      real_signal,
                                      t0 - slice_t0,
                                      t1 - slice_t0);
            // Analyze the slice
            auto spectrum(buf.template get<C>(1, plan.fftsize));
#if GABORATOR_USE_REAL_FFT
            plan.rft->transform(slice.data(), spectrum.data());
#else
            // Real to complex
            auto signal(buf.template get<C>(0, plan.fftsize));
            std::copy(slice.data(), slice.data() + plan.fftsize, signal.begin());
            plan.ft->transform(signal.data(), spectrum.data());
#endif
            auto tmp(buf.template get<C>(2, plan.sftsize_max));
            auto coefbuf(buf.template get<C>(4, plan.sftsize_max));

            T scale_factor = (T)params.coef_scale * plan.inv_fftsize_t;

            for (unsigned int obno = 0; obno < z.bandparams.size(); obno++) {
                band_params<T> *bp = z.bandparams[obno].get();
                C *sdata = tmp.data();

                // Multiply a slice of the spectrum by the frequency-
                // domain window and store in sdata.
                //
                // We need to take care not to overrun the beginning or
                // end of the spectrum - for the dc band, we always
                // need to wrap around to negative frequencies, and
                // potentially it could happen with other bands, too,
                // if they are really wide.  To avoid the overhead of
                // checking in the inner loop, use a separate slow path
                // for the rare cases where wrapping happens.

                int start_index = bp->fq_offset_int;
                int end_index = bp->fq_offset_int + bp->sftsize;
                if (start_index >= 0 && end_index < (int)((plan.fftsize >> 1) + 1)) {
                    // Fast path: the slice lies entirely within the
                    // positive-frequency half of the spectrum (including
                    // DC and Nyquist).
                    elementwise_product(sdata,
                                        spectrum.data() + start_index,
                                        bp->kernel.data(),
                                        bp->sftsize);
                } else {
                    // Slow path
                    for (size_t i = 0; i < bp->sftsize; i++)
                        sdata[i] = get_real_spectrum_coef(spectrum.data(),
                            (int)(start_index + i), plan.fftsize) * bp->kernel[i];
                }
                // The band center frequency is at the center of the
                // spectrum slice and at the center of the window, so
                // it also ends up at the center of sdata.  The center
                // frequency of the band is considered f=0, so for the
                // ifft, it should be at index 0, not the center.
                // Therefore, in principle we should perform an
                // ifftshift of sdata here before the ifft, but since
                // the time-domain data are going to be multiplied by
                // the shift kernel anyway, the ifftshift is baked
                // into the shift kernel by flipping the sign of every
                // other element so that it is effectively free.

                // Switch to time domain
                auto band(buf.template get<C>(3, plan.sftsize_max));
                bp->sft->itransform(sdata, band.data());

                // Apply ifftshift, adjust for non-integer center
                // frequency, correct phase, scale amplitude, and add
                // to the output coefficients.
                double ff = bp->center * plan.inv_fftsize_double;
                double arg;
                if (params.phase == phase_convention::absolute)
                    arg = -tau * (slice_t0 - plan.fat_size) * ff;
                else
                    arg = 0;
                C phase_times_scale = C(cos(arg), sin(arg)) * scale_factor;
                elementwise_product_times_scalar(coefbuf.data(), band.data(),
                                                 bp->shift_kernel.data(),
                                                 phase_times_scale,
                                                 bp->sftsize);
                // log2 of the coefficient downsampling factor
                int coef_shift = bp->step_log2;
                assert(((slice_t0 - (int)plan.fat_size) & ((1 << coef_shift) - 1)) == 0);
                coef_index_t ii = (slice_t0 - (int)plan.fat_size) >> coef_shift;

                // Only part of coefbuf contains substantially nonzero
                // data: that corresponding to the signal interval
                // t0..t1 + the actual support of the filter for this band.
                // There's no point adding the zeros to the coefficients,
                // so trim.
                coef_index_t ii0 = std::max(ii, (t0 - (int)bp->time_support) >> coef_shift);
                coef_index_t ii1 = std::min(ii + bp->sftsize,
                                            ((t1 + (int)bp->time_support) >> coef_shift) + 1);
                add(msc, bno_merge(oct, obno), ii0, ii1, coefbuf.data() + (ii0 - ii));
            }

            // Downsample
            if (oct + 1 < (int) n_octaves) {
                T *downsampled_dst = downsampled.data() +
                    slice_i * (plan.filet_size >> 1);

                auto sdata(buf.template get<C>(2, plan.sftsize_max));
                // This is using a larger buffer than we actually need
                auto ddata(buf.template get<C>(0, plan.sftsize_max));
                assert(ddata.size() >= plan.dsparams.sftsize);
                // Extract the low-frequency part of "spectrum" into "sdata"
                // and multiply it by the lowpass filter frequency response.
                // This means both positive and negative low frequencies.
                size_t half_size = plan.dsparams.sftsize >> 1;
                assert(plan.fftsize - half_size == 3 * half_size);
#if GABORATOR_USE_REAL_FFT
                // Positive frequencies
                elementwise_product(sdata.data(), spectrum.data(),
                                    plan.dsparams.kernel.data() + half_size,
                                    half_size);
                // Nyquist
                sdata[half_size] = 0;
                // Use the same buffer as the complex FFT, but as floats
                T *real_ddata = reinterpret_cast<T *>(ddata.data());
                plan.dsparams.rsft->itransform(sdata.data(), real_ddata);
                // Only accumulate nonzero part
                size_t n = ((slice_t1 - slice_t0) + 2 * (int)plan.fat_size + 1) >> 1;
                for (size_t i = 0; i < n; i++)
                    downsampled_dst[i] += real_ddata[i];
#else
                // Positive frequencies
                elementwise_product(sdata.data(), spectrum.data(),
                                    plan.dsparams.kernel.data() + half_size,
                                    half_size);
                // Negative requencies
                elementwise_product(sdata.data() + half_size,
                                    spectrum.data() + plan.fftsize - half_size,
                                    plan.dsparams.kernel.data(), half_size);
                // Convert to time domain
                plan.dsparams.sft->itransform(sdata.data(), ddata.data());
                for (unsigned int i = 0; i < plan.dsparams.sftsize; i++)
                    downsampled_dst[i] += ddata[i].real();
#endif
            }

            // Next slice
            slice_t0 = slice_t1;
        }

        // Recurse
        if (oct + 1 < (int)n_octaves)
            analyze_sliced(buf, oct + 1, downsampled.data() + (dst0a - dst0),
                           dst0a, dst1a, msc);
    }

    // Resynthesize audio from the coefficients in "msc".  The audio will
    // cover samples from t0 (inclusive) to t1 (exclusive), and is stored
    // starting at *real_signal, which must have room for (t1 - t0)
    // samples.  The octave "oct" is 0 except in recursive calls.

    void
    synthesize_sliced(int oct, const coefs<T> &msc,
                      sample_index_t t0, sample_index_t t1,
                      T *real_signal) const
    {
        const plan &plan(choose_plan(syn_plans, t1 - t0));

        // XXX clean up - no need to pass support arg
        slice_index_t si0 = plan.affected_slice_b(t0, plan.oct_support);
        slice_index_t si1 = plan.affected_slice_e(t1, plan.oct_support);

        // sub_signal holds the reconstructed subsampled signal from
        // the lower octaves, for the entire time interval covered by
        // the slices
        int sub_signal_len = ((si1 - si0) * plan.filet_size + 2 * plan.fat_size) / 2;
        pod_vector<T> sub_signal(sub_signal_len);
        std::fill(sub_signal.begin(), sub_signal.end(), 0);
        if (oct + 1 < (int)n_octaves) {
            int64_t sub_t0 = si0 * (plan.filet_size / 2);
            int64_t sub_t1 = sub_t0 + sub_signal_len;
            // Recurse
            assert(sub_t1 - sub_t0 == (int64_t)sub_signal.size());
            synthesize_sliced(oct + 1, msc, sub_t0, sub_t1, sub_signal.data());
        }

        // Allocate buffers for synthesize_one_slice(), to be shared
        // between successive calls to avoid repeated allocation
        pod_vector<C> buf0(plan.fftsize);
        //pod_vector<C> buf1(fftsize);
        pod_vector<C> buf2(plan.sftsize_max);
        pod_vector<C> buf3(plan.sftsize_max);

        pod_vector<T> downsampled(plan.dsparams.sftsize);
        pod_vector<T> signal_slice(plan.fftsize);

        // For each slice
        for (slice_index_t si = si0; si < si1; si++) {
            sample_index_t slice_t0 = si * plan.filet_size;

            // Copy downsampled signal to "downsampled" for upsampling
            if (oct + 1 < (int) n_octaves) {
                int bi = (si - si0) * filet_part(plan.dsparams.sftsize);
                int ei = bi + plan.dsparams.sftsize;
                assert(bi >= 0);
                assert(ei <= (int)sub_signal.size());
                std::copy(sub_signal.begin() + bi,
                          sub_signal.begin() + ei,
                          downsampled.begin());
            }

            synthesize_one_slice(oct, plan, msc, downsampled, slice_t0,
                                 signal_slice.data(), buf0, buf2, buf3);

            // Copy overlapping part
            sample_index_t b = std::max(slice_t0 + plan.fat_size, t0);
            sample_index_t e = std::min(slice_t0 + plan.fftsize - plan.fat_size, t1);
            for (sample_index_t i = b; i < e; i++)
                real_signal[i - t0] = signal_slice[i - slice_t0];
        }
    }

public:
    // The main analysis entry point.
    // The resulting coefficients are added to any existing
    // coefficients in "msc".

    void analyze(const T *real_signal, sample_index_t t0, sample_index_t t1,
                 coefs<T> &msc, int n_threads = 1) const
    {
        analyze1(real_signal, t0, t1, msc, n_threads, 1);
    }

    void analyze1(const T *real_signal, sample_index_t t0, sample_index_t t1,
                  coefs<T> &msc, int n_threads, int level) const
    {
        assert(msc.octaves.size() == n_octaves);
        (void)n_threads;
        buffers<T> buf(fftsize_max, sftsize_max);
        analyze_sliced(buf, 0, real_signal, t0, t1, msc);
    }

    // The main synthesis entry point

    void
    synthesize(const coefs<T> &msc, sample_index_t t0, sample_index_t t1,
               T *real_signal, int n_threads = 1) const
    {
        assert(params.synthesis);
        (void)n_threads;
        synthesize_sliced(0, msc, t0, t1, real_signal);
    }

    bool bno_split(int gbno, int &oct, unsigned int &obno, bool dc) const {
        return gaborator::bno_split(*cmeta_any, gbno, oct, obno, dc);
    }

    int bno_merge(int oct, unsigned int obno) const {
        return gaborator::bno_merge(*cmeta_any, oct, obno);
    }

    // Get the bounds of the range of existing coefficients for all bands,
    // in units of signal samples.
    void get_coef_bounds(const coefs<T> &msc,
                         sample_index_t &si0_ret, sample_index_t &si1_ret)
        const
    {
        // The greatest coefficient range typically occurs in the
        // lowest bandpass band, but this is not always the case,
        // so to be certain, check them all.
        sample_index_t min_si0 = INT64_MAX;
        sample_index_t max_si1 = INT64_MIN;
        for (int band = bands_begin(); band != bands_end(); band++) {
            coef_index_t ci0, ci1;
            get_band_coef_bounds(msc, band, ci0, ci1);
            // Convert from coefficient samples to signal samples
            int exp = band_scale_exp(band);
            sample_index_t si0 = shift_left(ci0, exp);
            sample_index_t si1 = shift_left(ci1 - 1, exp) + 1;
            min_si0 = std::min(min_si0, si0);
            max_si1 = std::max(max_si1, si1);
        }
        si0_ret = min_si0;
        si1_ret = max_si1;
    }

    unsigned int band_step_log2(int gbno) const {
        return gaborator::band_step_log2(*cmeta_any, gbno);
    }

    int bandpass_bands_begin() const { return 0; }
    int bandpass_bands_end() const { return n_bands_total - 1; }

    int bands_begin() const { return 0; }
    int bands_end() const { return n_bands_total; }

    int band_lowpass() const { return n_bands_total - 1; }
    int band_ref() const { return ffref_gbno; }

    // Get the center frequency of band number gbno as a fractional
    // frequency.  gbno must be a valid band number.  For the lowpass
    // band, this returns zero.
    double band_ff(int gbno) const {
        if (gbno == band_lowpass())
            return 0;
        return bandpass_band_ff(gbno);
    }

    ~analyzer() {
    }

    // Get the base 2 logarithm of the downsampling factor of
    // band "obno" in octave "oct"
    int band_scale_exp(int oct, unsigned int obno) const {
        return gaborator::band_scale_exp(*cmeta_any->octaves[oct].z, oct, obno);
    }

    // Get the base 2 logarithm of the downsampling factor of
    // band "gbno"
    int band_scale_exp(int gbno) const {
        int oct;
        unsigned int obno; // Band number within octave
        bool r = bno_split(gbno, oct, obno, true);
        assert(r);
        return band_scale_exp(oct, obno);
    }

    // Get the base 2 logarithm of the highest downsampling factor of
    // any band
    int band_scale_exp_max() const {
        return band_scale_exp(bandpass_bands_end() - 1);
    }


    // Find the sample time of the band "gbno" coefficient closest to
    // time "t".  "gbno" must be a valid band number.
    sample_index_t nearest_coef_sample(int gbno, double t) const {
        int shift = band_step_log2(gbno);
        return shift_left((sample_index_t) round(ldexp(t, -shift)), shift);
    }
    // Find the highest coefficient sample time less than or equal to
    // "t" for band "gbno".  "gbno" must be a valid band number.
    sample_index_t floor_coef_sample(int gbno, double t) const {
        int shift = band_step_log2(gbno);
        return shift_left((sample_index_t) floor(ldexp(t, -shift)), shift);
    }
    // Find the lowestt coefficient sample time greater than or equal
    // to "t" for band "gbno".  "gbno" must be a valid band number.
    sample_index_t ceil_coef_sample(int gbno, double t) const {
        int shift = band_step_log2(gbno);
        return shift_left((sample_index_t) ceil(ldexp(t, -shift)), shift);
    }

    // Members initialized in the constructor, and listed in
    // order of initialization
    parameters params;
    double band_spacing_log2;
    double band_spacing;
    double tuning_log2ff;
    double band0_log2ff;
    linear_transform log2ff_bandpass_band;
    linear_transform bandpass_band_log2ff;
    unsigned int n_bandpass_bands_total;

    struct plan: public refcounted {
        plan(const plan &) = delete;
        plan(analyzer<T> *anl, bool synthesis_, unsigned int fftsize_, double support_):
            ok(false),
            synthesis(synthesis_),
            fftsize(fftsize_),
            max_step_log2(0),
            oct_support(support_),
            sftsize_max(0)
        {
            fftsize_log2 = whichp2(fftsize);

            inv_fftsize_double = 1.0 / fftsize;
            inv_fftsize_t = (T) inv_fftsize_double;

#if GABORATOR_USE_REAL_FFT
            rft = pool<rfft<C *>, int>::shared.get(fftsize);
#else
            ft = pool<fft<C *>, int>::shared.get(fftsize);
#endif

            // Band number starting at 0 close to fs/2 and increasing
            // with decreasing frequency
            int tbno = 0;
            // Band number of the fs/8 band in the current octave
            int base = 0;
            int zno = 0;
            // Loop over the octaves, from high to low frequencies,
            // creating new zones where needed
            for (;;) {
                int max_bands_this_octave = (zno == 0) ?
                    anl->params.bands_per_octave * 2 : anl->params.bands_per_octave;
                base += max_bands_this_octave;
                int bands_remaining = anl->n_bandpass_bands_total - tbno;
                int bands_this_octave = std::min(max_bands_this_octave, bands_remaining);
                int bands_below = bands_remaining - bands_this_octave;
                bool dc_zone = (bands_below == 0);
                bool dc_adjacent_zone = (bands_below < (int)anl->params.bands_per_octave);
                if (zno < 2 || dc_zone || dc_adjacent_zone) {
                    make_zone(anl, zno, base - (tbno + bands_this_octave),
                              base - tbno, dc_zone, bands_below);
                    zno++;
                }
                octaves.push_back(octave<T>());
                octaves.back().z = zones[zno - 1].get();
                tbno += bands_this_octave;
                if (dc_zone)
                    break;
            }
            assert(octaves.size() == anl->n_octaves);

            // Set up the downsampling parameters in dsparams.

            // Downsampling is always by a factor of two.
            // dsparams.sftsize is the size of the FFT used to go back to
            // the time domain after discarding the top half of the
            // spectrum.
            dsparams.sftsize = fftsize >> 1;
            dsparams.kernel.resize(dsparams.sftsize);
            if (synthesis)
                dsparams.dual_kernel.resize(dsparams.sftsize);

            // Use the convolution of a rectangle and a gaussian.
            // A piecewise function composed from two half-gaussians
            // joined by a horizontal y=1 segment is not quite smooth
            // enough.  Put the passband in the middle.
            gaussian_windowed_lowpass(anl->ds_ff, anl->ds_ff_sd,
                                      dsparams.kernel.begin(),
                                      dsparams.kernel.end(), true);
            if (synthesis) {
                // The dual_kernel field of the downsampling pseudo-band holds
                // the upsampling filter, identical to the downsampling filter
                // except for amplitude scaling.
                std::copy(dsparams.kernel.begin(), dsparams.kernel.end(),
                          dsparams.dual_kernel.begin());
            }
            // Prescale the downsampling filter
            scale_vector(dsparams.kernel, inv_fftsize_double);
            if (synthesis) {
                // Prescale the upsampling filter
                scale_vector(dsparams.dual_kernel, 1.0 / dsparams.sftsize);
            }
#if GABORATOR_USE_REAL_FFT
            dsparams.rsft = pool<rfft<C *>, int>::shared.get(dsparams.sftsize);
#else
            dsparams.sft = pool<fft<C *>, int>::shared.get(dsparams.sftsize);
#endif
            // It may be possible to reduce the size of the fat from 1/4
            // of the fftsize, but we need to keep things aligned with the
            // coefficients
            if (! synthesis) {
                unsigned int align = 1 << max_step_log2;
                fat_size = (oct_support + (align - 1)) & ~(align - 1);
                // There must be room for at least one signal sample in each
                // half of the FFT; it can't be all fat
                if (!(fat_size < (fftsize >> 1)))
                    return; // fail
            } else {
                fat_size = fftsize >> 2;
            }
            filet_size = fftsize - 2 * fat_size;

            // Initialize coefficient metadata
            cmeta = new coefs_meta;
            cmeta->n_octaves = anl->n_octaves;
            cmeta->n_bands_total = anl->n_bands_total;
            cmeta->bands_per_octave = anl->params.bands_per_octave;
            cmeta->slice_len = fftsize >> 1;
            cmeta->zones.resize(zones.size());
            for (unsigned int zi = 0; zi < zones.size(); zi++) {
                zone<T> *z = zones[zi].get();
                typename zone_coefs_meta::band_vector bv(z->bandparams.size());
                for (unsigned int i = 0; i < z->bandparams.size(); i++) {
                    bv[i].slice_len = z->bandparams[i]->sftsize >> 1;
                    bv[i].slice_len_log2 = z->bandparams[i]->sftsize_log2 - 1;
                    bv[i].step_log2 = fftsize_log2 - z->bandparams[i]->sftsize_log2;
                }
                cmeta->zones[zi].init(bv);
            }
            cmeta->octaves.resize(octaves.size());
            tbno = 0;
            for (unsigned int i = 0; i < anl->n_octaves; i++) {
                cmeta->octaves[i].z = &cmeta->zones[this->octaves[i].z->zno];
                cmeta->octaves[i].n_bands_above = tbno;
                tbno += cmeta->octaves[i].z->bands.size();
            }
            ok = true;
        }

        void make_zone(analyzer<T> *anl, unsigned int zno, int bb, int be, bool dc_zone, int bands_below) {
            assert(zones.size() == zno);
            zone<T> *z = new zone<T>();
            z->zno = zno;
            zones.push_back(ref<zone<T> >(z));

            // The maximum number of mock bands to insert between the DC
            // band and the real bands.
            int n_mock_bands = 6;

            if (dc_zone) {
                // This zone goes all the way to DC
                band_params<T> *dc_band = make_band(anl, bb - 1, true);
                push(z->bandparams, dc_band);
            } else {
                // There are other zones below this; add mock bands
                // to simulate them for purposes of calculating the dual
                if (n_mock_bands > bands_below)
                    n_mock_bands = bands_below;
                // Mock DC band
                push(z->mock_bandparams,
                     make_band(anl, bb - 1 - n_mock_bands, true));
                // Mock bandpass bands
                for (int i = bb - 1; i > bb - 1 - n_mock_bands; i--)
                    push(z->mock_bandparams, make_band(anl, i, false));
            }

            // The actual bandpass bands of this zone
            for (int i = bb; i < be; i++)
                push(z->bandparams, make_band(anl, i, false));

            // If there are other zones above this, add mock bands
            // to simulate them for purposes of calculating the dual
            for (int i = be; i < (int)anl->params.bands_per_octave * 2; i++)
                push(z->mock_bandparams, make_band(anl, i, false));

            z->n_bands = (unsigned int)z->bandparams.size();

            // Find the largest coefficient step
            unsigned int m = 0;
            for (unsigned int obno = 0; obno < z->bandparams.size(); obno++) {
                m = std::max(m, z->bandparams[obno]->step_log2);
            }
            z->max_step_log2 = m;
            max_step_log2 = std::max(max_step_log2, m);

            // Calculate complex exponentials for non-integer center
            // frequency adjustment and phase convention adjustment
            for (unsigned int obno = 0; obno < z->bandparams.size(); obno++) {
                band_params<T> *bp = z->bandparams[obno].get();
                for (unsigned int i = 0; i < bp->sftsize; i++) {
                    double center =
                        (anl->params.phase == phase_convention::absolute) ? bp->center : 0;
                    double arg = tau * ((double)i / bp->sftsize) * -(center - bp->icenter);
                    C t(cos(arg), sin(arg));
                    // Apply ifftshift of spectrum in time domain
                    if (i & 1)
                        t = -t;
                    bp->shift_kernel[i] = t;
                }
            }

            if (synthesis) {
                // Accumulate window power for calculating dual
                z->power.resize(fftsize);
                std::fill(z->power.data(), z->power.data() + fftsize, (T)0);
                for (unsigned int obno = 0; obno < z->bandparams.size(); obno++)
                    accumulate_power(z->bandparams[obno].get(), z->power.data());
                for (unsigned int obno = 0; obno < z->mock_bandparams.size(); obno++)
                    accumulate_power(z->mock_bandparams[obno].get(), z->power.data());

                // Calculate duals
                for (unsigned int obno = 0; obno < z->bandparams.size(); obno++) {
                    band_params<T> *bp = z->bandparams[obno].get();
                    for (unsigned int i = 0; i < bp->sftsize; i++) {
                        // ii = large-FFT bin number
                        int ii = i + bp->fq_offset_int;
                        bp->dual_kernel[i] = bp->kernel[i] / z->power[ii & (fftsize - 1)];
                    }
                    for (unsigned int i = 0; i < bp->sftsize; i++) {
                        C t = bp->shift_kernel[i];
                        // Undo time-domain ifftshift
                        if (i & 1)
                            t = -t;
                        bp->shift_kernel_conj[i] = conj(t);
                    }
                }
            }

            // Free the mock band parameters
            z->mock_bandparams.clear();
            z->power.clear();
        }

        // Add the power of the kernel in "*bp" to "power"
        void
        accumulate_power(band_params<T> *bp, T *power) {
            for (unsigned int i = 0; i < bp->sftsize; i++) {
                // ii = large-FFT bin number
                unsigned int ii = (i + bp->fq_offset_int) & (fftsize - 1);
                assert(ii >= 0 && ii < fftsize);
                T y = bp->kernel[i];
                T p = y * y;
                power[ii] += p;
                if (! bp->dc) {
                    unsigned int ni = fftsize - ii;
                    ni &= fftsize - 1; // XXX is this correct?
                    assert(ni < fftsize);
                    power[ni] += p;
                }
            }
        }

        // Calculate band parameters for a single band.
        //
        // rbno is a "relative band number" indicating a frequency
        // within its octave: it is 0 for the fs/8 band and increases
        // with frequency.  It is not always the same as the index
        // into its octaves' bandparams[] or the coefficient bands
        // (those would be called obno, not rbno).
        //
        // If dc is true, this is the DC band, and rbno indicates
        // the cutoff frequency; it is one less than the rbno of
        // the lowest-frequency bandpass band.

        band_params<T> *
        make_band(analyzer *anl, double rbno, bool dc) {
            if (dc)
                // Make the actual DC band cutoff frequency a bit higher,
                // by an empirically chosen fraction of a band, to reduce
                // power fluctuations.
                rbno += 0.8750526596806952;

            // For bandpass bands, the center frequency, or
            // for the DC band, the lowpass cutoff frequency,
            // as a fractional frequency.
            double ff = anl->rbno_ff(rbno);

            // Standard deviation of the bandpass Gaussian,
            // as a fractional frequency
            double ffsd = anl->ff_sd(ff);
            // The support of the Gaussian, i.e., the smallest standard
            // deviation at which it can be truncated on each side
            // without the error exceeding our part of the error budget,
            // which is some fraction of params.max.  Note
            // that this is one-sided; the full width of the support
            // is 2 * ff_support.
            double ff_support = gaussian_support(ffsd, anl->params.max_error * 0.5);
            // Additional support for the flat portion of the DC band lowpass
            double dc_support = dc ? ff : 0;
            // The support as the number of FFT frequency bands needed,
            // allowing for both sides of the Gaussian.
            int fq_2support = int(ceil((ff_support + dc_support) * 2 * fftsize));

            unsigned int sftsize = next_power_of_two(fq_2support);

            // If there is a very small number of bandpass bands, the
            // taper of the lowpass band may extend past the Nyquist
            // frequency, causing sftsize to become greater than fftsize.
            // This would mean sampling the lowpass band at a higher sample
            // rate than the original signal, which would be a bit silly,
            // and having sftsize > fftsize will break things by causing
            // shift amounts to go negative, so instead we force sftsize
            // = fftsize but remember the original sftsize.  The lowpass
            // kernel is calculated using the original sftsize and allowed
            // to alias over the truncated sftsize.
            unsigned int orig_sftsize = sftsize;
            if (sftsize > fftsize) {
                assert(dc);
                sftsize = fftsize;
            }

            band_params<T> *bp = new band_params<T>;
            bp->dc = dc;
            bp->sftsize = sftsize;
            bp->sftsize_log2 = whichp2(bp->sftsize);
            bp->step_log2 = fftsize_log2 - bp->sftsize_log2;
            bp->step = 1U << bp->step_log2;
            sftsize_max = std::max(sftsize_max, bp->sftsize);
            bp->sft = pool<fft<C *>, int>::shared.get(bp->sftsize);

            bp->kernel.resize(bp->sftsize);
            bp->shift_kernel.resize(bp->sftsize);
            if (synthesis) {
                bp->dual_kernel.resize(bp->sftsize);
                bp->shift_kernel_conj.resize(bp->sftsize);
            }

            if (dc)
                bp->center = 0;
            else
                bp->center = ff * fftsize;
            bp->icenter = (int)rint(bp->center);
            bp->ff = ff;
            bp->fq_offset_int = bp->icenter - (bp->sftsize >> 1);
            bp->ffsd = ffsd;

            // Calculate frequency-domain window kernel

            if (dc) {
                // The cutoff frequency is a fraction of the fftsize, but
                // gaussian_windowed_lowpass() designs the filter in terms
                // of the the sftsize, so we need to scale the frequencies
                // accordingly.  The cast to double is necessary to get
                // a float division in pathological cases where orig_sftsize
                // is larger than fftsize.
                double scale = (double)fftsize / orig_sftsize;
                pod_vector<T> win(orig_sftsize);
                gaussian_windowed_lowpass(
                    ff * scale,
                    ffsd * scale,
                    win.begin(), win.end());
                for (unsigned int i = 0; i < bp->kernel.size(); i++)
                    bp->kernel[i] = 0;
                for (unsigned int i = 0; i < win.size(); i++)
                    bp->kernel[i & (bp->kernel.size() - 1)] += win[i];
                fftshift(bp->kernel.begin(), bp->kernel.end());
            } else {
                for (unsigned int i = 0; i < bp->sftsize; i++) {
                    // ii = large-FFT band number
                    unsigned int ii = i + bp->fq_offset_int;
                    // this_ff = fractional frequency of this kernel sample
                    double this_ff = ii * inv_fftsize_double;
                    T y = norm_gaussian(ffsd, this_ff - ff);
                    bp->kernel[i] = y;
                }
            }

            double time_sd = sd_f2t(ffsd);
            double time_support =
                gaussian_support(time_sd, anl->params.max_error);
            bp->time_support = ceil(time_support);
            return bp;
        }

        // Index of first slice affected by sample at t0

        // fft number i covers the sample range
        // t = (i * filetsize .. i * filetsize + (fftsize - 1))
        // t >= i * filetsize and t < i * filetsize + fftsize
        // A sample at t affects ffts i where
        //   i <= t / filetsize and
        //   i > (t - fftsize) / filetsize
        // the filet of fft number i covers the sample range
        // (fat + (i * filetsize) .. fat + (i * filetsize) + (filetsize - 1))
        //
        // However, due to the FFT size being rounded up to a power of two,
        // the outermost parts have near-zero weights and can be ignored;
        // this is done by adjusting the time values by the width of that
        // outermost part, which is (fatsize - support)

        slice_index_t affected_slice_b(sample_index_t t0, unsigned int support) const {
            return floor_div(t0 - fftsize + (fat_part(fftsize) - support), filet_part(fftsize)) + 1;
        }

        // Index of first slice not affected by sample at t1
        slice_index_t affected_slice_e(sample_index_t t1, unsigned int support) const {
            return floor_div(t1 - 1 - (fat_part(fftsize) - support), filet_part(fftsize)) + 1;
        }

        bool ok;
        bool synthesis;

        unsigned int fftsize_log2; // log2(fftsize)
        unsigned int fftsize; // The size of the main FFT, a power of two.
        unsigned int fat_size;
        unsigned int filet_size;
        unsigned int max_step_log2;

        // The width of the widest filter in the time domain, in
        // octave subsamples
        unsigned int oct_support;

        double inv_fftsize_double; // 1.0 / fftsize
        T inv_fftsize_t; // 1.0f / fftsize (if using floats)

        unsigned int sftsize_max; // The size of the largest band FFT, a power of two
        std::vector<ref<zone<T> > > zones;
        downsampling_params<T> dsparams;

        // Fourier transform object for transforming a full slice
#if GABORATOR_USE_REAL_FFT
        rfft<C *> *rft;
#else
        fft<C *> *ft;
#endif
        std::vector<octave<T> > octaves; // Per-octave parameters

        ref<coefs_meta> cmeta; // Coefficient metadata
    };

    std::vector<ref<plan>> anl_plans;
    std::vector<ref<plan>> syn_plans;

    unsigned int n_octaves;
    unsigned int n_bands_total; // Total number of frequency bands, including DC

    double top_band_log2ff; // log2 of fractional frequency of the highest-frequency band
    int ffref_gbno; // Band number of the reference frequency

    double ds_ff; // Downsampling filter -6 dB transition frequency
    double ds_ff_sd; // Downsampling filter standard deviation
    double ds_time_support; // Downsampling filter time-domain kernel support, each side

    // The largest time-domain support of any bandpass filter,
    // in octave subsamples.  This is determined by the "band0"
    // filter (near fs/8).

    double band0_time_analysis_support;
    double band0_time_synthesis_support;

    unsigned int fftsize_max; // Largest FFT size of any plan
    unsigned int sftsize_max; // Largest SFT size of any plan

    ref<coefs_meta> cmeta_any;
};


// Iterate over the slices of a row (band) having slice length
// 2^sh that contain coefficients with indices ranging from i0
// (inclusive) to i1 (exclusive), and call the function f for
// each such slice (full or partial), with the arguments
//
//  sli - slice index
//  bvi - index of first coefficient to process within the slice
//  len - number of coefficients to process within the slice

template <class F>
void foreach_slice(unsigned int sh, coef_index_t i0, coef_index_t i1, F f) {
    // Note that this can be called with i0 > i1 and needs to handle
    // that case gracefully.
    // Band size (power of two)
    int bsize = 1 << sh;
    // Adjust for t=0 being outside the filet
    int fatsize = bsize >> 1;
    i0 -= fatsize;
    i1 -= fatsize;
    coef_index_t i = i0;
    while (i < i1) {
        // Slice index
        slice_index_t sli = i >> sh;
        // Band vector index
        unsigned int bvi = i & (bsize - 1);
        unsigned int len = bsize - bvi;
        unsigned int remain = (unsigned int)(i1 - i);
        if (remain < len)
            len = remain;
        f(sli, bvi, len);
        i += len;
    }
}

// As foreach_slice, but call the "process_existing_slice" method of
// the given "dest" object for each full or partial slice of
// coefficients, and/or the "process_missing_slice" method for each
// nonexistent slice.
//
// Template parameters:
//   T is the spectrogram value type
//   D is the dest object type
//   C is the coefficient type

template <class T, class D, class C = complex<T> >
struct row_foreach_slice {
    typedef C value_type;
    row_foreach_slice(const coefs<T, C> &msc,
                      int oct_, unsigned int obno_):
        oct(oct_), obno(obno_), sc(msc.octaves[oct]),
        sh(sc.meta->bands[obno].slice_len_log2)
    {
        assert(oct < (int)msc.octaves.size());
    }
public:
    void operator()(coef_index_t i0, coef_index_t i1, D &dest) const {
        foreach_slice(sh, i0, i1,
            [this, &dest](slice_index_t sli, unsigned int bvi, unsigned int len) {
                oct_coefs<C> *c = get_existing_coefs(sc, sli);
                if (c) {
                    dest.process_existing_slice(c->bands[obno] + bvi, len);
                } else {
                    dest.process_missing_slice(len);
                }
            });
    }
    int oct;
    unsigned int obno;
    const sliced_coefs<C> &sc;
    unsigned int sh;
};

// Helper class for row_source

template <class C, class OI>
struct writer_dest {
    writer_dest(OI output_): output(output_) { }
    void process_existing_slice(C *bv, size_t len) {
        // Can't use std::copy here because it takes the output
        // iterator by value, and using the return value does not
        // work, either.
        for (size_t i = 0; i < len; i++)
            *output++ = bv[i];
    }
    void process_missing_slice(size_t len) {
        for (size_t i = 0; i < len; i++)
            *output++ = C();
    }
    OI output;
};

// Retrieve a sequence of coefficients from a row (band) in the
// spectrogram, with indices ranging from i0 to i1.  The indices can
// be negative, and can extend outside the available data, in which
// case zero is returned.  The coefficients are written through the
// output iterator "output".
// Template arguments:
//   T is the spectrogram value type
//   OI is the output iterator type
//   C is the coefficient value type
// Note defaults for template arguments defined in forward declaration.

template <class T, class OI, class C>
struct row_source {
    row_source(const coefs<T, C> &msc_,
               int oct_, unsigned int obno_):
        slicer(msc_, oct_, obno_)
    { }
    OI operator()(coef_index_t i0, coef_index_t i1, OI output) const {
        writer_dest<C, OI> dest(output);
        slicer(i0, i1, dest);
        return dest.output;
    }
    row_foreach_slice<T, writer_dest<C, OI>, C> slicer;
};

// The opposite of row_source: store a sequence of coefficients into
// a row (band) in the spectrogram.  This duplicates quite a lot of
// the row_source code above (without comments); the main part that's
// different is marked by the comments "Begin payload" and "End
// payload".  Other differences: iterator is called II rather than OI,
// and the coefs are not const.

template <class T, class II, class C>
struct row_dest {
    typedef C value_type;
    row_dest(coefs<T, C> &msc,
             int oct_, unsigned int obno_):
        oct(oct_), obno(obno_), sc(msc.octaves[oct]),
        sh(sc.meta->bands[obno].slice_len_log2)
    {
        assert(oct < (int)msc.octaves.size());
    }
public:
    II operator()(coef_index_t i0, coef_index_t i1, II input) const {
        assert(i0 <= i1);
        int bsize = 1 << sh;
        int fatsize = bsize >> 1;
        i0 -= fatsize;
        i1 -= fatsize;
        coef_index_t i = i0;
        while (i < i1) {
            slice_index_t sli = i >> sh;
            unsigned int bvi = i & (bsize - 1);
            unsigned int len = bsize - bvi;
            unsigned int remain = (unsigned int)(i1 - i);
            if (remain < len)
                len = remain;
            int bvie = bvi + len;
            // Begin payload
            oct_coefs<C> *c = &get_or_create_coefs(sc, sli);
            C *bv = c->bands[obno];
            for (int j = bvi; j < bvie; j++)
                bv[j] = *input++;
            i += len;
            // End payload
        }
        return input;
    }
    int oct;
    unsigned int obno;
    sliced_coefs<C> &sc;
    unsigned int sh;
};

// One more set of duplicated code, now for adding to coefficients

template <class T, class II, class C>
struct row_add_dest {
    typedef C value_type;
    row_add_dest(coefs<T, C> &msc,
                 int oct_, unsigned int obno_):
        oct(oct_), obno(obno_), sc(msc.octaves[oct]),
        sh(sc.meta->bands[obno].slice_len_log2)
    {
        assert(oct < (int)msc.octaves.size());
    }
public:
    II operator()(coef_index_t i0, coef_index_t i1, II input) const {
        assert(i0 <= i1);
        int bsize = 1 << sh;
        int fatsize = bsize >> 1;
        i0 -= fatsize;
        i1 -= fatsize;
        coef_index_t i = i0;
        while (i < i1) {
            slice_index_t sli = i >> sh;
            unsigned int bvi = i & (bsize - 1);
            unsigned int len = bsize - bvi;
            unsigned int remain = (unsigned int)(i1 - i);
            if (remain < len)
                len = remain;
            int bvie = bvi + len;
            // Begin payload
            oct_coefs<C> *c = &get_or_create_coefs(sc, sli);
            C *bv = c->bands[obno];
            for (int j = bvi; j < bvie; j++)
                bv[j] += *input++;
            i += len;
            // End payload
        }
        return input;
    }
    int oct;
    unsigned int obno;
    sliced_coefs<C> &sc;
    unsigned int sh;
};

// Helper for process() below.  Here, the function f() operates on an
// array of consecutive coefficient samples rather than a single
// sample.

template <class T, class F, class C0, class... CI>
void apply_to_slice(bool create,
                    F f,
                    int b0, // = INT_MIN
                    int b1, // = INT_MAX
                    sample_index_t st0, // = INT64_MIN
                    sample_index_t st1, // = INT64_MAX
                    coefs<T, C0>& coefs0,
                    coefs<T, CI>&... coefsi)
{
    b0 = std::max(b0, 0);
    b1 = std::min(b1, (int)coefs0.meta->n_bands_total);
    for (int band = b0; band < b1; band++) {
        int oct;
        unsigned int obno;
        bool valid = gaborator::bno_split(*coefs0.meta, band, oct, obno, true);
        assert(valid);

        int exp = coefs0.meta->octaves[oct].z->bands[obno].step_log2 + oct;
        int time_step = 1 << exp;

        coef_index_t ci0 = (st0 + time_step - 1) >> exp;
        coef_index_t ci1 = ((st1 - 1) >> exp) + 1;
        if (! create) {
            // Restrict to existing coefficient index range
            coef_index_t cib0, cib1;
            get_band_coef_bounds(coefs0, oct, obno, cib0, cib1);
            ci0 = std::max(ci0, cib0);
            ci1 = std::min(ci1, cib1);
        }
        unsigned int sh = coefs0.meta->octaves[oct].z->bands[obno].slice_len_log2;
        sample_index_t st = shift_left(ci0, exp);
        foreach_slice(sh, ci0, ci1,
                      [&](slice_index_t sli, unsigned int bvi,
                          unsigned int len)
        {
            oct_coefs<C0> *c = create ?
                &get_or_create_coefs(coefs0.octaves[oct], sli) :
                get_existing_coefs(coefs0.octaves[oct], sli);
            if (c) {
                // p0 points to coefficient from the first set
                C0 *p0 = c->bands[obno] + bvi;
                f(band, st, time_step, len, p0,
                  get_or_create_coefs(coefsi.octaves[oct], sli).bands[obno] + bvi...);
            }
            st += len * time_step;
        });
    }
}

// Common implementation of process() and fill()

template <class T, class F, class C0, class... CI>
void apply_common(bool create,
                  F f,
                  int b0, // = INT_MIN
                  int b1, // = INT_MAX
                  sample_index_t st0, // = INT64_MIN
                  sample_index_t st1, // = INT64_MAX
                  coefs<T, C0> &coefs0,
                  coefs<T, CI>&... coefsi)
{
    apply_to_slice(create,
                   [&](int bno, int64_t st, int time_step,
                       unsigned len, C0 *p0, CI *...pi)
    {
        for (unsigned int i = 0; i < len; i++) {
            f(bno, st, *p0++, *pi++...);
            st += time_step;
        }
    }, b0, b1, st0, st1, coefs0, coefsi...);
}

// Iterate over one or more coefficient sets in parallel and apply the
// function f, passing it a coefficient from each set as an argument.
//
// The application can be optionally limited to coefficients within
// the band range b0 to b1 and/or the sample time range st0 to st1.
//
// The first coefficient set ("channel 0") is treated specially; it
// determines which coefficients are iterated over (optionally further
// restricted by b0/b1/st0/st1).  That is, the iteration is over the
// coefficients that already exist in channel 0, and no new
// coefficients will be allocated in channel 0.  In the other
// channels, new coefficients will be created on demand when they are
// missing from that channel but present in channel 0.
//
// The coefficients may be of a different data type in each set.
//
// The arguments to f() are:
//
//    int bno        Band number
//    int64_t st     Sample time
//    C &p0          Coefficient from first set
//    C... &pi       Coefficient from subsequent set

template <class T, class F, class C0, class... CI>
void process(F f,
             int b0, // = INT_MIN
             int b1, // = INT_MAX
             sample_index_t t0, // = INT64_MIN
             sample_index_t t1, // = INT64_MAX
             coefs<T, C0> &coefs0,
             coefs<T, CI>&... coefsi)
{
    apply_common(false, f, b0, b1, t0, t1, coefs0, coefsi...);
}

template <class T, class F, class C0, class... CI>
void fill(F f,
          int b0, // = INT_MIN
          int b1, // = INT_MAX
          sample_index_t t0, // = INT64_MIN
          sample_index_t t1, // = INT64_MAX
          coefs<T, C0> &coefs0,
          coefs<T, CI>&... coefsi)
{
    apply_common(true, f, b0, b1, t0, t1, coefs0, coefsi...);
}

// Apply the function f to each existing coefficient in the
// coefficient set msc within the time range st0 to st1.  The
// initial analyzer argument is ignored.
//
// The arguments to f() are:
//
//    C &c           Coefficient
//    int b          Band number
//    int64_t t      Time in samples
//
// This is for backwards compatibility; process() is now preferred.

template <class T, class F>
void apply(const analyzer<T> &, coefs<T> &msc, F f,
           sample_index_t st0 = INT64_MIN,
           sample_index_t st1 = INT64_MAX)
{
    process([&](int b, int64_t t, complex<T>& c) {
            f(c, b, t);
        }, INT_MIN, INT_MAX, st0, st1, msc);
}

template <class T>
void forget_before(const analyzer<T> &, coefs<T> &msc,
                   sample_index_t limit, bool clean_cut = false)
{
    typedef complex<T> C;
    unsigned int n_oct = (unsigned int) msc.octaves.size();
    for (unsigned int oct = 0; oct < n_oct; oct++) {
        sliced_coefs<C> &sc = msc.octaves[oct];
        // Convert limit from samples to slices, rounding down.
        // This assumes all bands in the octave have the same
        // time range, as they must.
        // First convert samples to coefficients, rounding down
        int obno = 0; // Any band would do, and band 0 always exists
        zone_coefs_meta *zmeta = msc.meta->octaves[oct].z;
        coef_index_t ci = limit >> band_scale_exp(*zmeta, oct, obno);
        // Then convert coefficients to slices, accounting for
        // fat and rounding down
        unsigned int slice_len = zmeta->bands[obno].slice_len;
        unsigned int slice_len_log2 = zmeta->bands[obno].slice_len_log2;
        int fat = slice_len >> 1;
        slice_index_t sli = (ci - fat) >> slice_len_log2;
        sc.slices.erase_before(sli);
        if (clean_cut) {
            // Partially erase slice at boundary, if any
            const ref<oct_coefs<C> > *t = sc.slices.get(sli);
            if (! t)
                continue;
            if (! *t)
                continue;
            const oct_coefs<C> &c = **t;
            unsigned int n_bands = (unsigned int)c.bands.size();
            for (unsigned int obno = 0; obno < n_bands; obno++) {
                C *band = c.bands[obno];
                unsigned int len = sc.meta->bands[obno].slice_len;
                sample_index_t st = sample_time(*sc.meta, sli, 0, oct, obno);
                int time_step = 1 << band_scale_exp(*sc.meta, oct, obno);
                for (unsigned int i = 0; i < len; i++) {
                    if (st < limit)
                        band[i] = 0;
                    else
                        break;
                    st += time_step;
                }
            }
        }
    }
}


} // namespace

#endif
