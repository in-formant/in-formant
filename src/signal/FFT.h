//
// Created by rika on 11/10/2019.
//

#ifndef SPEECH_ANALYSIS_FFT_H
#define SPEECH_ANALYSIS_FFT_H

#include <Eigen/Core>

namespace FFT {

    void realTransform(Eigen::ArrayXd & data, int isign);

}

#endif //SPEECH_ANALYSIS_FFT_H
