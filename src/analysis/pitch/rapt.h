#ifndef ANALYSIS_PITCH_RAPT_H
#define ANALYSIS_PITCH_RAPT_H

#include "rpcxx.h"
#include "../linpred/linpred.h"

namespace Analysis {

    class RAPT {
    public:
        RAPT();

        double computeFrame(const double *data, int length, double sampleRate);
        rpm::vector<double> computePath();

        double F0min;    // minimum F0 to search for (Hz)                | 50
        double F0max;    // maximum F0 to search for (Hz)                | 500
        double cand_tr;  // minimum acceptable peak value in NCCF        | 0.3
        double lag_wt;   // linear lag taper factor for NCCF             | 0.3
        double freq_wt;  // cost factor for F0 change                    | 0.02
        double vtran_c;  // fixed voicing-state transition cost          | 0.005
        double vtr_a_c;  // delta amplitude modulated transition cost    | 0.5
        double vtr_s_c;  // delta spectrum modulated transition cost     | 0.5
        double vo_bias;  // bias to encourage voiced hypotheses          | 0.0
        double doubl_c;  // cost of exact F0 doubling of halving         | 0.35
        double a_fact;   // term to decrease PHI of weak signals         | 10000
        int   n_cands;  // max. number of hypotheses at each frame      | 20

        struct Cand {
            double localCost; // local cost of candidate for dynamic programming
            bool voiced;     // suspected voiced candidate or not
            double L;         // lag of peak for voiced candidate
            double C;         // value of peak
            double rms;       // RMS of candidate
        };
    
        struct Frame {
            rpm::vector<Cand> cands;
           
            double Fs;

            double rms;
            double rr;

            rpm::vector<double> ar;
            double S;
        };

        LP::Autocorr lpc;
        int lpcOrder;
        int nbFrames;

    private:
        rpm::deque<Frame> frames;
    };
}

#endif // ANALYSIS_PITCH_RAPT_H
