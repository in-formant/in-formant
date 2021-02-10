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
        gaborator::analyzer<double> analyzer;
        gaborator::coefs<double> coefs;
        double fs;
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

        rpm::vector<SpectrogramCoefs>& getSpectrogramCoefs();

        TimeTrack<double>& getPitchTrack();

        TimeTrack<double>& getFormantTrack(int i);
        int getFormantTrackCount() const;
        void setFormantTrackCount(int n);

    private:
        int mTrackLength;

        std::shared_mutex mMutex;
        int mCatchupCount;

        TimeTrack<rpm::vector<double>>                mSoundTrack;

        double mTime;

        rpm::vector<SpectrogramCoefs> mSpectrogramCoefs;
        
        TimeTrack<double> mPitchTrack;
        rpm::vector<TimeTrack<double>> mFormantTracks;
    };

}

#endif // MAIN_CONTEXT_DATA_STORE_H

