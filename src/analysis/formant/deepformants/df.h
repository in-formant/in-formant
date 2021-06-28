#ifndef DEEP_FORMANTS_H
#define DEEP_FORMANTS_H

#include <Eigen/Dense>

#ifdef ENABLE_TORCH 

#undef slots
#undef ERROR
#pragma warning(push, 0)
#ifdef _WIN32
#endif
#include <torch/script.h>
#pragma warning(pop)
#define slots Q_SLOTS

#define MAX_AMPLITUDE_16BIT (32767.0)

template<typename Derived>
Eigen::ArrayXd build_feature_row(const Eigen::ArrayBase<Derived>& x);

struct DFModelHolder {
public:
    torch::jit::script::Module *torchModule();

    static DFModelHolder *instance();
    static void initialize();

private:
    torch::jit::script::Module mTorchModule;

    static std::unique_ptr<DFModelHolder> sInstance;
};

#endif

#endif // DEEP_FORMANTS_H
