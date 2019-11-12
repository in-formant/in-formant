//
// Created by rika on 14/09/2019.
//

#ifndef SPEECH_ANALYSIS_WINDOW_H
#define SPEECH_ANALYSIS_WINDOW_H

#include <Eigen/Core>

namespace Window {

    Eigen::ArrayXd createHamming(int size);
    Eigen::ArrayXd createHanning(int size);
    Eigen::ArrayXd createBlackmanHarris(int size);
    Eigen::ArrayXd createGaussian(int size);
    Eigen::ArrayXd createKaiser(int size);

}

#endif //SPEECH_ANALYSIS_WINDOW_H
