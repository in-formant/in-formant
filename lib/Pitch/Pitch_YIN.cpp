#include "Pitch.h"
#include "Yin/YIN.h"

using namespace Eigen;

void Pitch::estimate_YIN(const ArrayXd & x, double fs, Pitch::Estimation & result, double threshold)
{
    ArrayXd diff = YIN::difference(x);
    ArrayXd cmnd = YIN::cumulative_mean_normalized_difference(diff);
    int tau = YIN::absolute_threshold(cmnd, threshold);

    if (tau != -1) {
        result.isVoiced = true;
        result.pitch = fs / std::get<0>(YIN::parabolic_interpolation(cmnd, tau));
    }
    else {
        result.isVoiced = false;
        result.pitch = -1;
    }
}
