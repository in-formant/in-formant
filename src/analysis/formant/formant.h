#ifndef ANALYSIS_FORMANT_H
#define ANALYSIS_FORMANT_H

#include "rpcxx.h"
#include "../../modules/audio/resampler/resampler.h"

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
        virtual ~FormantSolver() {}
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
            void setFrameAudio(const double *x, int length, double fs);
        private:
            Module::Audio::Resampler rs;
            rpm::vector<double> xv;
            double fs;
        };
    };

};

#endif // ANALYSIS_FORMANT_H
