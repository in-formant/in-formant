#include "df.h"
#include <QFile>

std::unique_ptr<DFModelHolder> DFModelHolder::sInstance(nullptr);

torch::jit::script::Module *DFModelHolder::torchModule() {
    return &mTorchModule;
}

DFModelHolder *DFModelHolder::instance() {
    if (sInstance == nullptr) {
        throw std::runtime_error("DeepFormants: ModelHolder singleton instance was not initialized!");
    }
    return sInstance.get();
}

void DFModelHolder::initialize() {
    if (sInstance != nullptr) {
        throw std::runtime_error("DeepFormants: ModelHolder singleton instance can only be initialized once!");
    }

    DFModelHolder *holder = new DFModelHolder;

    try {
        QFile file(":/model.pt");
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray buffer = file.readAll();
            std::string data(buffer.data(), buffer.size());
            std::istringstream stream(data);
            holder->mTorchModule = torch::jit::load(stream, c10::kCPU);
            file.close();
        }
    }
    catch (const c10::Error& e) {
        throw std::runtime_error("DeepFormants: Error loading the model.");
    }

    sInstance.reset(holder);
}