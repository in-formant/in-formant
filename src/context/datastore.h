#ifndef MAIN_CONTEXT_DATA_STORE_H
#define MAIN_CONTEXT_DATA_STORE_H

#include "../fixedsizevector.h"
#include "../analysis/analysis.h"
#include <array>
#include <shared_mutex>

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

        FixedSizeVector<std::vector<std::array<double, 2>>>& getSoundSpectrumTrack();
        FixedSizeVector<std::vector<std::array<double, 2>>>& getLpSpectrumTrack();
        FixedSizeVector<double>&                             getPitchTrack();
        FixedSizeVector<std::vector<Analysis::FormantData>>& getFormantTrack();
        FixedSizeVector<std::vector<double>>&                getSoundTrack();
        FixedSizeVector<std::vector<double>>&                getGifTrack();

    private:
        int mTrackLength;

        std::shared_mutex mMutex;
        int mCatchupCount;

        FixedSizeVector<std::vector<std::array<double, 2>>> mSoundSpectrumTrack;
        FixedSizeVector<std::vector<std::array<double, 2>>> mLpSpectrumTrack;
        FixedSizeVector<double>                             mPitchTrack;
        FixedSizeVector<std::vector<Analysis::FormantData>> mFormantTrack;
        FixedSizeVector<std::vector<double>>                mSoundTrack;
        FixedSizeVector<std::vector<double>>                mGifTrack;
    };

}

#endif // MAIN_CONTEXT_DATA_STORE_H

