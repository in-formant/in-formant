#include "DeepFormants/df.h"
#include "../Analyser.h"

using namespace Eigen;

void Analyser::analyseDeepFormants() {

    ArrayXd features = build_feature_row(x);
    
    ArrayXd result = predictFromFeatures(features);

    lastFormantFrame.nFormants = 4;
    lastFormantFrame.formant.resize(4);
    for (int i = 0; i < 4; ++i) {
        lastFormantFrame.formant[i].frequency = 1000 * result(i);
    }
}
