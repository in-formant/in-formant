#ifndef ANALYSIS_PITCH_H
#define ANALYSIS_PITCH_H

#include <vector>
#include <cstdint>
#include <memory>

#include "../fft/fft.h"

namespace Analysis {

    struct PitchResult {
        float pitch;
        bool voiced;
    };

    class PitchSolver {
    public:
        virtual ~PitchSolver() {}
        virtual PitchResult solve(const float *data, int length, int sampleRate) = 0;
    };

    namespace Pitch {
        class AMDF_M : public PitchSolver {
        public:
            AMDF_M(float minPitch, float maxPitch, float alpha);
            PitchResult solve(const float *data, int length, int sampleRate) override;
        private:
            float mMinPitch;
            float mMaxPitch;
            float mAlpha;
            std::vector<float> mAMDF;
            std::vector<uint32_t> m1bAMDF;
            std::vector<float> m1bACF;
        };

        class Yin : public PitchSolver {
        public:
            Yin(float threshold);
            PitchResult solve(const float *data, int length, int sampleRate) override;
        private:
            float mThreshold;
            std::shared_ptr<ComplexFFT> mFFT;
            std::vector<double> mAutocorrelation;
            std::vector<double> mDifference;
            std::vector<double> mCMND;
        };
    }

}
#endif // ANALYSIS_PITCH_H
