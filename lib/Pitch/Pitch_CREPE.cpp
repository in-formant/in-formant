#include "Pitch.h"
#include "CREPE/crepe.h"

using namespace Eigen;

void Pitch::estimate_CREPE(const ArrayXd& x, double fs, Pitch::Estimation& est)
{
    if (fs != 16000) {
        est.pitch = 0;
        est.isVoiced = false;
        return;
    }

    ArrayXd input = x - x.mean();
    
    double stdDev = std::sqrt((input - input.mean()).square().sum()) / (input.size() - 1);
    input /= stdDev;

    ArrayXd activation = get_activation(input);
    
    int center;
    double confidence = activation.maxCoeff(&center);

    if (confidence > 0.6) {
        double cents = to_local_average_cents(activation, center);
        est.pitch = cents_to_hertz(cents);
        est.isVoiced = est.pitch != 0;
    }
    else {
        est.pitch = 0;
        est.isVoiced = false;
    }
}
