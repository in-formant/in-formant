#ifndef WAVELET_WAVELET_H
#define WAVELET_WAVELET_H

#include "wt.h"
#include <algorithm>
#include <vector>
#include <stdexcept>

namespace Analysis {

template <typename data_t>
rpm::vector<data_t> swt(const rpm::vector<data_t>& _data, const Wt::DiscreteWavelet& wavelet, int level, int startLevel, bool trimApprox = false)
{
    rpm::vector<data_t> data(_data);

    if (data.size() % 2 == 1)
        throw std::runtime_error("Length of data must be even");
    
    if (data.size() < 1)
        throw std::runtime_error("Data must have non-zero size");
    
    if (level < 1)
        throw std::runtime_error("Level value must be greater than zero");

    int endLevel = startLevel + level;
    int maxLevel = Wt::swt_max_level(data.size());
    
    if (startLevel >= maxLevel)
        throw std::runtime_error("Start level is too high");
    
    if (endLevel > maxLevel)
        throw std::runtime_error("Level value is too high");

    int outputLen = (int) Wt::swt_buffer_length((int) data.size());

    if (outputLen < 1)
        throw std::runtime_error("Invalid output length");
   
    rpm::vector<data_t> cA, cD;
    int retval;

    rpm::vector<data_t> ret;

    for (int i = startLevel + 1; i < endLevel + 1; ++i) {
        int dataSize = (int) data.size();

        if constexpr (std::is_same<data_t, double>::value) {
            cD.resize(outputLen, data_t(0.0));
            retval = Wt::double_swt_d(&data[0], dataSize, &wavelet, &cD[0], outputLen, i);
            if (retval < 0) {
                throw std::runtime_error("SWT failed");
            }
        }
        else if constexpr (std::is_same<data_t, std::complex<double>>::value) {
            cD.resize(outputLen, data_t(0.0));
            retval = Wt::double_complex_swt_d(&data[0], dataSize, &wavelet, &cD[0], outputLen, i);
            if (retval < 0) {
                throw std::runtime_error("SWT failed");
            }
        }

        if constexpr (std::is_same<data_t, double>::value) {
            cA.resize(outputLen, data_t(0.0));
            retval = Wt::double_swt_a(&data[0], dataSize, &wavelet, &cA[0], outputLen, i);
            if (retval < 0) {
                throw std::runtime_error("SWT failed");
            }
        }
        else if constexpr (std::is_same<data_t, std::complex<double>>::value) {
            cA.resize(outputLen, data_t(0.0));
            retval = Wt::double_complex_swt_a(&data[0], dataSize, &wavelet, &cA[0], outputLen, i);
            if (retval < 0) {
                throw std::runtime_error("SWT failed");
            }
        }

        data = cA;
        if (!trimApprox) {
            ret.insert(ret.end(), cA.begin(), cA.end());
            ret.insert(ret.end(), cD.begin(), cD.end());
        }
        else {
            ret.insert(ret.end(), cD.begin(), cD.end());
        }
    }

    if (trimApprox) {
        ret.insert(ret.end(), cA.begin(), cA.end());
    }

    std::reverse(ret.begin(), ret.end());

    return ret;
}

}

#endif // WAVELET_WAVELET_H
