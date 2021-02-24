#ifndef MAIN_CONTEXT_DATA_STORE_H
#define MAIN_CONTEXT_DATA_STORE_H

#include "rpcxx.h"
#include "../timetrack.h"
#include "../analysis/analysis.h"
#include <array>
#include <shared_mutex>
#include <functional>
#include <gaborator/gaborator.h>

namespace Main {

    struct SpectrogramCoefs {
        Eigen::VectorXd amplitudes;
        double duration;
        double minFrequency;
        double maxFrequency;
    };

    class DataStore {
    public:
        DataStore();

        void beginWrite();
        void endWrite();

        int beginRead();
        void endRead();

        TimeTrack<rpm::vector<double>>& getSoundTrack();
    
        double getTime() const;
        void setTime(double t);

        TimeTrack<SpectrogramCoefs>& getSpectrogram();

        OptionalTimeTrack<double>& getPitchTrack();

        OptionalTimeTrack<double>& getFormantTrack(int i);
        int getFormantTrackCount() const;
        void setFormantTrackCount(int n);

    private:
        int mTrackLength;

        std::shared_mutex mMutex;
        int mCatchupCount;

        TimeTrack<rpm::vector<double>> mSoundTrack;

        double mTime;

        TimeTrack<SpectrogramCoefs> mSpectrogram;
        
        OptionalTimeTrack<double> mPitchTrack;
        rpm::vector<OptionalTimeTrack<double>> mFormantTracks;
    };

}

#endif // MAIN_CONTEXT_DATA_STORE_H

