#ifndef ANALYSIS_FORMANT_H
#define ANALYSIS_FORMANT_H

#include "rpcxx.h"
#include "../../modules/audio/resampler/resampler.h"

#ifdef _WIN32
extern "C" void __assert_fail(const char* expr, const char *filename, unsigned int line, const char *assert_func) noexcept;
#endif

#undef slots
#undef ERROR
#pragma warning(push, 0)
#ifdef _WIN32
#endif
#include <torch/script.h>
#pragma warning(pop)
#define slots Q_SLOTS

namespace Analysis {

    struct FormantData {
        double frequency;
        double bandwidth;
    };

    struct FormantResult {
        rpm::vector<FormantData> formants;
    };

    class FormantSolver {
    public:
        virtual FormantResult solve(const double *lpc, int lpcOrder, double sampleRate) = 0;
    };

    namespace Formant {
        class SimpleLP : public FormantSolver {
        public:
            FormantResult solve(const double *lpc, int lpcOrder, double sampleRate) override;
        };

        class FilteredLP : public FormantSolver {
        public:
            FormantResult solve(const double *lpc, int lpcOrder, double sampleRate) override;
        };
        
        struct KarmaState;

        class Karma : public FormantSolver {
        public:
            Karma();
            ~Karma();
            FormantResult solve(const double *lpc, int lpcOrder, double sampleRate) override;
        private:
            KarmaState *state;
        };

        class DeepFormants : public FormantSolver {
        public:
            DeepFormants();
            FormantResult solve(const double *lpc, int lpcOrder, double sampleRate) override;
            void setFrameAudio(const rpm::vector<double>& x);
        private:
            torch::jit::script::Module module;
            rpm::vector<double> xv;
            double fs;
        };
    };

};

#endif // ANALYSIS_FORMANT_H
