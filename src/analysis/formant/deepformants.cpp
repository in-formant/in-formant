#include "formant.h"
#include "deepformants/df.h"
#include "../util/util.h"
#include <algorithm>

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
    ArrayXd result = predictFromFeatures(features);

    FormantResult formantResult;
    for (int i = 0; i < 4; ++i) {
        formantResult.formants.push_back({
            .frequency = 1000 * result(i),
            .bandwidth = 80,
        });
    }

    return formantResult;
}
