#include "df.h"
#ifdef _WIN32
#  define __assert_fail(...) // Suppress warning.
#endif
#include <torch/script.h>

#ifdef QT_CORE_LIB
#  include <QFile>
#endif

static torch::jit::script::Module dfModule;
static bool dfModuleLoaded = false;

static void loadModuleFromFile(QFile& file)
{
    try {
        QByteArray buffer = file.readAll();
        std::string data(buffer.data(), buffer.size());
        std::istringstream stream(data);
        dfModule = torch::jit::load(stream, c10::kCPU);
    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading the model: " << e.msg() << std::endl;
        return;
    }
}

static void loadModule()
{
#ifdef QT_CORE_LIB
    QFile file(":/model.pt");
    if (file.open(QIODevice::ReadOnly)) {
        loadModuleFromFile(file);
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

    torch::Tensor output = dfModule.forward({input}).toTensor();

    return Eigen::Map<Eigen::ArrayXf>(output.data_ptr<float>(), output.size(1)).cast<double>();
}
