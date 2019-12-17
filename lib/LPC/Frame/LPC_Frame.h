//
// Created by rika on 12/10/2019.
//

#ifndef SPEECH_ANALYSIS_LPC_FRAME_H
#define SPEECH_ANALYSIS_LPC_FRAME_H

#include <Eigen/Core>

namespace Formant {
    struct Frame;
}

namespace LPC {

    struct Frame;

    namespace Huber {
        struct huber_s;
    }

    void filter(const Frame & lpc, Eigen::ArrayXd & x);
    void filterInverse(const Frame & lpc, Eigen::ArrayXd & x);

    bool frame_auto(const Eigen::ArrayXd & sound, Frame & lpc);
    bool frame_covar(const Eigen::ArrayXd & sound, Frame & lpc);
    bool frame_burg(const Eigen::ArrayXd & sound, Frame & lpc);

    void frame_huber(const Eigen::ArrayXd & sound, const Frame & lpc1, Frame & lpc2, Huber::huber_s & hs);

    void toFormantFrame(const Frame & lpc, Formant::Frame & frm, double samplingFrequency);

}

#endif //SPEECH_ANALYSIS_LPC_FRAME_H
