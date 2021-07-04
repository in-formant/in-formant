#include "df.h"
#include <QFile>

DFModelHolder *DFModelHolder::sInstance;

DFModelHolder::DFModelHolder()
    : mDctN(0), mFft1N(0), mFft2N(0)
{
}

Analysis::ReReFFT *DFModelHolder::dct(int n)
{
    if (mDctN != n) {
        mDct.reset(new Analysis::ReReFFT(n, FFTW_REDFT10));
        mDctN = n;
    }
    return mDct.get();
}

Analysis::RealFFT *DFModelHolder::fft1(int n)
{
    if (mFft1N != n) {
        mFft1.reset(new Analysis::RealFFT(n));
        mFft1N = n;
    }
    return mFft1.get();
}

Analysis::RealFFT *DFModelHolder::fft2(int n)
{
    if (mFft2N != n) {
        mFft2.reset(new Analysis::RealFFT(n));
        mFft2N = n;
    }
    return mFft2.get();
}

torch::jit::script::Module *DFModelHolder::torchModule() {
    return &mTorchModule;
}

DFModelHolder *DFModelHolder::instance() {
    if (sInstance == nullptr) {
        throw std::runtime_error("DeepFormants: ModelHolder singleton instance was not initialized!");
    }
    return sInstance;
}

void DFModelHolder::initialize(DFModelHolder **pptr) {
    if (sInstance != nullptr) {
        throw std::runtime_error("DeepFormants: ModelHolder singleton instance can only be initialized once!");
    }

    *pptr = new DFModelHolder;

    try {
        QFile file(":/model.pt");
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray buffer = file.readAll();
            std::string data(buffer.data(), buffer.size());
            std::istringstream stream(data);
            (*pptr)->mTorchModule = torch::jit::load(stream, c10::kCPU);
            file.close();
        }
    }
    catch (const c10::Error& e) {
        throw std::runtime_error("DeepFormants: Error loading the model.");
    }

    sInstance = *pptr;
}