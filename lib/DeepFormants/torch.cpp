#ifdef HAS_ML_FORMANTS 

#include "df.h" 

torch::jit::script::Module dfModule;

void loadModuleFromBuffer(char* buffer, int size)
{
    std::istringstream in(std::string(buffer, size));
    
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        dfModule = torch::jit::load(in);
    }
    catch (const c10::Error& e) {
        std::cerr << "error loading the model: " << std::endl << e.msg() << std::endl;
        return;
    }

    std::cout << "Loaded model" << std::endl;
}

Eigen::ArrayXd predictFromFeatures(const Eigen::ArrayXd& features) 
{
    at::Tensor input = torch::ones({1, 350});

    for (int i = 0; i < 350; ++i) {
        input[0][i] = features(i);
    }

    at::Tensor output = dfModule.forward({input}).toTensor();

    return Eigen::Map<Eigen::ArrayXf>(output.data_ptr<float>(), output.size(1)).cast<double>();
}

#endif // HAS_ML_FORMANTS
