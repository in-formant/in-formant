#ifndef ANALYSIS_PITCH_RAPT_H
#define ANALYSIS_PITCH_RAPT_H

#include "../../modules/audio/resampler/resampler.h"
#include "../linpred/linpred.h"
#include <deque>
#include <vector>

namespace Analysis {

    class RAPT {
    public:
        RAPT();

        float computeFrame(const float *data, int length, float sampleRate);
        std::vector<float> computePath();

        float F0min;    // minimum F0 to search for (Hz)                | 50
        float F0max;    // maximum F0 to search for (Hz)                | 500
        float cand_tr;  // minimum acceptable peak value in NCCF        | 0.3
        float lag_wt;   // linear lag taper factor for NCCF             | 0.3
        float freq_wt;  // cost factor for F0 change                    | 0.02
        float vtran_c;  // fixed voicing-state transition cost          | 0.005
        float vtr_a_c;  // delta amplitude modulated transition cost    | 0.5
        float vtr_s_c;  // delta spectrum modulated transition cost     | 0.5
        float vo_bias;  // bias to encourage voiced hypotheses          | 0.0
        float doubl_c;  // cost of exact F0 doubling of halving         | 0.35
        float a_fact;   // term to decrease PHI of weak signals         | 10000
        int   n_cands;  // max. number of hypotheses at each frame      | 20

        struct Cand {
            float localCost; // local cost of candidate for dynamic programming
            bool voiced;     // suspected voiced candidate or not
            float L;         // lag of peak for voiced candidate
            float C;         // value of peak
            float rms;       // RMS of candidate
        };
    
        struct Frame {
            std::vector<Cand> cands;
           
            float Fs;

            float rms;
            float rr;

            std::vector<float> ar;
            float S;
        };

        LP::Autocorr lpc;
        int lpcOrder;
        int nbFrames;

    private:
        Module::Audio::Resampler downsampler;

        std::deque<Frame> frames;
    };

}

#endif // ANALYSIS_PITCH_RAPT_H
