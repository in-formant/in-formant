#ifndef DEEP_FORMANTS_H
#define DEEP_FORMANTS_H

#ifdef ENABLE_TORCH 

#include <Eigen/Dense>

#undef slots
#undef ERROR
#pragma warning(push, 0)
#ifdef _WIN32
#endif
#include <torch/script.h>
#pragma warning(pop)
#define slots Q_SLOTS

#include "../../fft/fft.h"

#define MAX_AMPLITUDE_16BIT (32767.0)

template<typename Derived>
Eigen::ArrayXd build_feature_row(const Eigen::ArrayBase<Derived>& x);

struct DFModelHolder {
public:
    DFModelHolder();

    torch::jit::script::Module *torchModule();

    Analysis::ReReFFT *dct(int n);
    Analysis::RealFFT *fft1(int n);
    Analysis::RealFFT *fft2(int n);

    static DFModelHolder *instance();
    static void initialize(DFModelHolder **pptr);

private:
    torch::jit::script::Module mTorchModule;
    
    int mDctN;
    std::unique_ptr<Analysis::ReReFFT> mDct;

    int mFft1N;
    std::unique_ptr<Analysis::RealFFT> mFft1;

    int mFft2N;
    std::unique_ptr<Analysis::RealFFT> mFft2;

    static DFModelHolder *sInstance;
};

#endif // ENABLE_TORCH

#endif // DEEP_FORMANTS_H
