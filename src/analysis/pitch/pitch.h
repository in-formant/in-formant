#ifndef ANALYSIS_PITCH_H
#define ANALYSIS_PITCH_H

#include "rpcxx.h"
#include <cstdint>
#include <memory>

#include "../fft/fft.h"
#include "rapt.h"

namespace Analysis {

    struct PitchResult {
        double pitch;
        bool voiced;
    };

    class PitchSolver {
    public:
        virtual ~PitchSolver() {}
        virtual PitchResult solve(const double *data, int length, int sampleRate) = 0;
    };

    namespace Pitch {
        /*
        class AMDF_M : public PitchSolver {
        public:
            AMDF_M(double minPitch, double maxPitch, double alpha);
            PitchResult solve(const double *data, int length, int sampleRate) override;
        private:
            double mMinPitch;
            double mMaxPitch;
            double mAlpha;
            rpm::vector<double> mAMDF;
            rpm::vector<uint32_t> m1bAMDF;
            rpm::vector<double> m1bACF;
        };
        */

        class Yin : public PitchSolver {
        public:
            Yin(double threshold);
            PitchResult solve(const double *data, int length, int sampleRate) override;
        private:
            double mThreshold;
            std::shared_ptr<ComplexFFT> mFFT;
            rpm::vector<double> mAutocorrelation;
            rpm::vector<double> mDifference;
            rpm::vector<double> mCMND;
        };

        class MPM : public PitchSolver {
        public:
            PitchResult solve(const double *data, int length, int sampleRate) override;
        };

        class RAPT : public PitchSolver, public Analysis::RAPT {
        public:
            RAPT();
            PitchResult solve(const double *data, int length, int sampleRate) override;
        private:
            rpm::vector<double> pitches;
        };

        /*
        class IRAPT : public PitchSolver {
        public:
            IRAPT();
            PitchResult solve(const double *data, int length, int sampleRate) override;
        };
        */
    }

}
#endif // ANALYSIS_PITCH_H
