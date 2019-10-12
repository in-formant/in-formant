//
// Created by rika on 11/10/2019.
//

#include "FFT.h"

#define FFT_DATA_TYPE double
#include "FFT_core.h"

using namespace Eigen;

struct FFT_Table {
    int n;
    ArrayXd trigcache;
    ArrayXi splitcache;
};

static void forwardRealFFT(ArrayXd & data);
static void reverseRealFFT(ArrayXd & data);
static void fftForward(FFT_Table & table, ArrayXd & data);
static void fftBackward(FFT_Table & table, ArrayXd & data);
static void fftTableInit(FFT_Table & table, int n);

void forwardRealFFT(ArrayXd & data)
{
    FFT_Table table;
    fftTableInit(table, data.size());
    fftForward(table, data);
}

void reverseRealFFT(ArrayXd & data)
{
    FFT_Table table;
    fftTableInit(table, data.size());
    fftBackward(table, data);
}

void fftForward(FFT_Table & table, ArrayXd & data)
{
    if (table.n == 1) return;

    drftf1(table.n,
            data.data(),
            table.trigcache.data(),
            table.trigcache.data() + table.n,
            table.splitcache.data());
}

void fftBackward(FFT_Table & table, ArrayXd & data)
{
    if (table.n == 1) return;

    drftb1(table.n,
            data.data(),
            table.trigcache.data(),
            table.trigcache.data() + table.n,
            table.splitcache.data());
}

void fftTableInit(FFT_Table & table, int n)
{
    table.n = n;
    table.trigcache.setZero(3 * n);
    table.splitcache.setZero(32);
    rffti(n, table.trigcache.data(), table.splitcache.data());
}

void FFT::realTransform(Eigen::ArrayXd & data, int isign)
{
    if (isign == 1)
        forwardRealFFT(data);
    else
        reverseRealFFT(data);
}