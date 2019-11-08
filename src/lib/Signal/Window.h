//
// Created by rika on 14/09/2019.
//

#ifndef SPEECH_ANALYSIS_WINDOW_H
#define SPEECH_ANALYSIS_WINDOW_H

#include <Eigen/Core>

namespace Window {

    void applyHamming(Eigen::ArrayXd & x);
    void applyHanning(Eigen::ArrayXd & x);

    void applyBlackmanHarris(Eigen::ArrayXd & x);

    Eigen::ArrayXd createGaussian(int size);

}

#endif //SPEECH_ANALYSIS_WINDOW_H
