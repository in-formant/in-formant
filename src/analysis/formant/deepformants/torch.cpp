#include "df.h"
#ifdef _WIN32
#  define __assert_fail(...) // Suppress warning.
#endif
#include <torch/torch.h>
#include "cnpy.h"

#ifdef QT_CORE_LIB
#  include <QFile>
#endif

namespace nn = torch::nn;

constexpr int D = 350;

struct DeepFormantsNet : nn::Module {
    DeepFormantsNet()
        : dense1(register_module("Dense1", nn::Linear(D, 1024))),
          dense2(register_module("Dense2", nn::Linear(1024, 512))),
          dense3(register_module("Dense3", nn::Linear(512, 256))),
          out(register_module("out", nn::Linear(256, 4)))
    {
    }

    torch::Tensor forward(torch::Tensor x) {
        x = torch::sigmoid(dense1(x));
        x = torch::sigmoid(dense2(x));
        x = torch::sigmoid(dense3(x));
        return out(x);
    }

    nn::Linear dense1{nullptr};
    nn::Linear dense2{nullptr};
    nn::Linear dense3{nullptr};
    nn::Linear out{nullptr};
};

DeepFormantsNet dfNet;

static bool dfModuleLoaded = false;

static void loadModuleFromBuffer(QFile& file)
{
    cnpy::npz_t stateDict = cnpy::npz_load(file);

    auto params = dfNet.named_parameters(true);
    auto buffers = dfNet.named_buffers(true);

    torch::autograd::GradMode::set_enabled(false);

    for (auto& [name, value] : stateDict) {
        rpm::vector<double> vec(value.as_vec<double>());
        
        rpm::vector<int64_t> sizes(value.shape.size());
        std::copy(value.shape.begin(), value.shape.end(), sizes.begin());

        auto* t = params.find(name);
        if (t != nullptr) {
            t->copy_(torch::tensor(torch::ArrayRef(vec)).resize_(sizes));
            std::cout << vec[2] << std::endl;
        } else {
            t = buffers.find(name);
            if (t != nullptr) {
                t->copy_(torch::tensor(torch::ArrayRef(vec)).resize_(sizes));
            }
        }
    }
    
    torch::autograd::GradMode::set_enabled(true);
}

static void loadModule()
{
#ifdef QT_CORE_LIB
    QFile file(":/model.npz");
    if (file.open(QIODevice::ReadOnly)) {
        loadModuleFromBuffer(file);
        file.close();
        dfModuleLoaded = true;
    }
#endif
}

Eigen::ArrayXd predictFromFeatures(const Eigen::ArrayXd& features) 
{
    if (!dfModuleLoaded) {
        loadModule();
    }

    torch::Tensor input = torch::ones({1, 350});

    for (int i = 0; i < 350; ++i) {
        input[0][i] = features(i);
    }

    torch::Tensor output = dfNet.forward({input});

    return Eigen::Map<Eigen::ArrayXf>(output.data_ptr<float>(), output.size(1)).cast<double>();
}
