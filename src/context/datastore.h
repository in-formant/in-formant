#ifndef MAIN_CONTEXT_DATA_STORE_H
#define MAIN_CONTEXT_DATA_STORE_H

#include "rpcxx.h"
#include "../timetrack.h"
#include "../analysis/analysis.h"
#include <array>
#include <chrono>
#include <shared_mutex>
#include <functional>

enum class FrequencyScale : unsigned int {
    Linear      = 0,
    Logarithmic = 1,
    Mel         = 2,
    ERB         = 3,
};

namespace Main {

    struct SpectrogramCoefs {
        Eigen::VectorXd magnitudes;
        double sampleRate;
    };

    class DataStore {
    public:
        DataStore();

        void beginWrite();
        void endWrite();

        void beginRead();
        void endRead();

        double getTime() const;
        void setTime(double t);

        double getRealTime() const;
        void startRealTime();
        void stopRealTime();

        TimeTrack<SpectrogramCoefs>& getSpectrogram();

        OptionalTimeTrack<double>& getPitchTrack();

        OptionalTimeTrack<double>& getFormantTrack(int i);
        int getFormantTrackCount() const;
        void setFormantTrackCount(int n);

        TimeTrack<rpm::vector<double>>& getSoundTrack();
        TimeTrack<rpm::vector<double>>& getGifTrack();
    
    private:
        int mTrackLength;

        std::shared_mutex mMutex;

        volatile double mTime;

        bool mIsRealTimeStarted;
        double mRealTimeOffset;
        std::chrono::time_point<std::chrono::high_resolution_clock> mRealTimeStart;

        TimeTrack<SpectrogramCoefs> mSpectrogram;
        
        OptionalTimeTrack<double> mPitchTrack;
        rpm::vector<OptionalTimeTrack<double>> mFormantTracks;

        TimeTrack<rpm::vector<double>> mSoundTrack;
        TimeTrack<rpm::vector<double>> mGifTrack;
    };

}

#endif // MAIN_CONTEXT_DATA_STORE_H

