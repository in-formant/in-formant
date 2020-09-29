#ifndef ANALYSIS_INVGLOT_H
#define ANALYSIS_INVGLOT_H

#include "../linpred/linpred.h"
#include "../fft/fft.h"
#include <memory>
#include <vector>
#include <Eigen/Dense>

namespace Analysis {

    struct InvglotResult {
        float sampleRate;
        std::vector<float> glotSig;
    };

    class InvglotSolver {
    public:
        virtual ~InvglotSolver() {}
        virtual InvglotResult solve(const float *x, int length, float sampleRate) = 0;
    };

    namespace Invglot {
        class IAIF : public InvglotSolver {
        public:
            IAIF(float d);
            InvglotResult solve(const float *x, int length, float sampleRate) override;
        private:
            std::unique_ptr<LinpredSolver> lpc;
            float d;
        };

        class AMGIF : public InvglotSolver {
        public:
            AMGIF(int J);
            InvglotResult solve(const float *x, int length, float sampleRate) override;
        private:
            void computeC();
            std::vector<Eigen::MatrixXd> C;
            IAIF iaif;
            RealFFT fft;
            int Jmax;
        };
    }

}

#endif // ANALYSIS_INVGLOT_H
