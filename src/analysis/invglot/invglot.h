#ifndef ANALYSIS_INVGLOT_H
#define ANALYSIS_INVGLOT_H

#include "../linpred/linpred.h"
#include "../fft/fft.h"
#include <memory>
#include <vector>
#include <Eigen/Dense>

namespace Analysis {

    struct InvglotResult {
        double sampleRate;
        std::vector<double> glotSig;
    };

    class InvglotSolver {
    public:
        virtual ~InvglotSolver() {}
        virtual InvglotResult solve(const double *x, int length, double sampleRate) = 0;
    };

    namespace Invglot {
        class IAIF : public InvglotSolver {
        public:
            IAIF(double d);
            InvglotResult solve(const double *x, int length, double sampleRate) override;
        private:
            std::unique_ptr<LinpredSolver> lpc;
            double d;
        };

        class GFM_IAIF : public InvglotSolver {
        public:
            GFM_IAIF(double d);
            InvglotResult solve(const double *x, int length, double sampleRate) override;
        private:
            std::unique_ptr<LinpredSolver> lpc;
            double d;
        };

        class AMGIF : public InvglotSolver {
        public:
            AMGIF(int J);
            InvglotResult solve(const double *x, int length, double sampleRate) override;
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
