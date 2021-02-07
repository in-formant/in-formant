#ifndef MAIN_CONTEXT_DATA_STORE_H
#define MAIN_CONTEXT_DATA_STORE_H

#include "rpcxx.h"
#include "../fixedsizevector.h"
#include "../timetrack.h"
#include "../analysis/analysis.h"
#include <array>
#include <shared_mutex>
#include <gaborator/gaborator.h>

namespace Main {

    class DataStore {
    public:
        DataStore();

        void setTrackLength(int trackLength);
        int getTrackLength() const;

        void beginWrite();
        void endWrite();

        int beginRead();
        void endRead();

        FixedSizeVector<rpm::vector<double>>& getSoundTrack();
    
        double getTime() const;
        void setTime(double t);

        std::optional<gaborator::analyzer<double>>& getSpectrogramAnalyzer();
        std::optional<gaborator::coefs<double>>& getSpectrogramCoefs();

        TimeTrack<double>& getPitchTrack();

        TimeTrack<double>& getFormantTrack(int i);
        int getFormantTrackCount() const;
        void setFormantTrackCount(int n);

    private:
        int mTrackLength;

        std::shared_mutex mMutex;
        int mCatchupCount;

        FixedSizeVector<rpm::vector<double>>                mSoundTrack;

        double mTime;

        std::optional<gaborator::analyzer<double>> mSpectrogramAnalyzer;
        std::optional<gaborator::coefs<double>> mSpectrogramCoefs;
        
        TimeTrack<double> mPitchTrack;
        rpm::vector<TimeTrack<double>> mFormantTracks;
    };

}

#endif // MAIN_CONTEXT_DATA_STORE_H

