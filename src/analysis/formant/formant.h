#ifndef ANALYSIS_FORMANT_H
#define ANALYSIS_FORMANT_H

#include <vector>

namespace Analysis {

    struct FormantData {
        float frequency;
        float bandwidth;
    };

    struct FormantResult {
        std::vector<FormantData> formants;
    };

    class FormantSolver {
    public:
        virtual ~FormantSolver() {}
        virtual FormantResult solve(const float *lpc, int lpcOrder, float sampleRate) = 0;
    };

    namespace Formant {
        class SimpleLP : public FormantSolver {
        public:
            FormantResult solve(const float *lpc, int lpcOrder, float sampleRate) override;
        };

        class FilteredLP : public FormantSolver {
        public:
            FormantResult solve(const float *lpc, int lpcOrder, float sampleRate) override;
        };
    };

};

#endif // ANALYSIS_FORMANT_H
