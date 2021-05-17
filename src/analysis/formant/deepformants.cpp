#include "formant.h"
#include "deepformants/df.h"
#include "../util/util.h"
#include <algorithm>
#include <QFile>

using namespace Analysis::Formant;
using Analysis::FormantResult;

using namespace Eigen;

DeepFormants::DeepFormants()
{
    try {
        QFile file(":/model.pt");
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray buffer = file.readAll();
            std::string data(buffer.data(), buffer.size());
            std::istringstream stream(data);
            module = torch::jit::load(stream, c10::kCPU);
            file.close();
        }
    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading the model: " << e.msg() << std::endl;
    }
}

void DeepFormants::setFrameAudio(const rpm::vector<double>& x)
{
    xv = x;
}

FormantResult DeepFormants::solve(const double *, int, double)
{
    Map<const ArrayXd> x(xv.data(), xv.size());

    ArrayXd features = build_feature_row(x);
    
    torch::Tensor input = torch::ones({1, 350});
    for (int i = 0; i < 350; ++i) {
        input[0][i] = features(i);
    }

    torch::Tensor output = module.forward({input}).toTensor();

    ArrayXd result = Map<ArrayXf>(output.data_ptr<float>(), output.size(1)).cast<double>();

    FormantResult formantResult;
    for (int i = 0; i < 4; ++i) {
        formantResult.formants.push_back({
            1000 * result(i),
            80,
        });
    }

    return formantResult;
}
