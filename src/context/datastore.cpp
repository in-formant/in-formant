#include "datastore.h"

using namespace Main;

DataStore::DataStore()
    : mTrackLength(0),
      mCatchupCount(0)
{
}

void DataStore::setTrackLength(int trackLength)
{
    mSoundSpectrumTrack.resize(trackLength);
    mLpSpectrumTrack.resize(trackLength);
    mPitchTrack.resize(trackLength);
    mFormantTrack.resize(trackLength);
    mSoundTrack.resize(trackLength);
    mGifTrack.resize(trackLength);
    mTrackLength = trackLength;
}

int DataStore::getTrackLength() const
{
    return mTrackLength;
}

void DataStore::beginWrite()
{
    mMutex.lock();
}

void DataStore::endWrite()
{
    mCatchupCount++;
    mMutex.unlock();
}

int DataStore::beginRead()
{
    mMutex.lock_shared();

    int catchup = 0;
    std::swap(mCatchupCount, catchup);
    return catchup;
}

void DataStore::endRead()
{
    mMutex.unlock_shared();
}

FixedSizeVector<std::vector<std::array<double, 2>>>& DataStore::getSoundSpectrumTrack()
{
    return mSoundSpectrumTrack;
}

FixedSizeVector<std::vector<std::array<double, 2>>>& DataStore::getLpSpectrumTrack()
{
    return mLpSpectrumTrack;
}

FixedSizeVector<double>& DataStore::getPitchTrack()
{
    return mPitchTrack;
}

FixedSizeVector<std::vector<Analysis::FormantData>>& DataStore::getFormantTrack()
{
    return mFormantTrack;
}

FixedSizeVector<std::vector<double>>& DataStore::getSoundTrack()
{
    return mSoundTrack;
}

FixedSizeVector<std::vector<double>>& DataStore::getGifTrack()
{
    return mGifTrack;
}
