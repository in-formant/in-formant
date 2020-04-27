#ifndef DEEP_FORMANTS_H
#define DEEP_FORMANTS_H

#ifdef HAS_ML_FORMANTS

#include <torch/script.h>
#include <Eigen/Dense>

extern torch::jit::script::Module dfModule;

void loadModuleFromBuffer(char* buffer, int size);
Eigen::ArrayXd predictFromFeatures(const Eigen::ArrayXd& features);

Eigen::ArrayXd feature_periodogram(const Eigen::ArrayXd& x, int nfft);
Eigen::ArrayXd feature_arspec(const Eigen::ArrayXd& x, int order, int nfft);

Eigen::ArrayXd arSpecs(const Eigen::ArrayXd& x, int order);
Eigen::ArrayXd specPS(const Eigen::ArrayXd& x);

Eigen::ArrayXd build_feature_row(const Eigen::ArrayXd& x);

#endif

#endif // DEEP_FORMANTS_H
