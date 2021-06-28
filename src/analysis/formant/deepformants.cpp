#include "formant.h"
#include "deepformants/df.h"
#include "../util/util.h"
#include <algorithm>
#include <QFile>

using namespace Analysis::Formant;
using Analysis::FormantResult;

using namespace Eigen;

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

    torch::Tensor output = DFModelHolder::instance()->torchModule()->forward({input}).toTensor();

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
