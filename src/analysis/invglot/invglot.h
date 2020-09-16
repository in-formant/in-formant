#ifndef ANALYSIS_INVGLOT_H
#define ANALYSIS_INVGLOT_H

#include "../linpred/linpred.h"
#include <memory>
#include <vector>

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
    }

}

#endif // ANALYSIS_INVGLOT_H
