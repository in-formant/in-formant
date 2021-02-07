#include "datastore.h"

using namespace Main;

DataStore::DataStore()
    : mTrackLength(0),
      mCatchupCount(1),
      mTime(0)
{
}

void DataStore::setTrackLength(int trackLength)
{
    mSoundTrack.resize(trackLength);
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

FixedSizeVector<rpm::vector<double>>& DataStore::getSoundTrack()
{
    return mSoundTrack;
}

double DataStore::getTime() const
{
    return mTime;
}

void DataStore::setTime(double t)
{
    mTime = t;
}

std::optional<gaborator::analyzer<double>>& DataStore::getSpectrogramAnalyzer()
{
    return mSpectrogramAnalyzer;
}

std::optional<gaborator::coefs<double>>& DataStore::getSpectrogramCoefs()
{
    return mSpectrogramCoefs;
}

TimeTrack<double>& DataStore::getPitchTrack()
{
    return mPitchTrack;
}

TimeTrack<double>& DataStore::getFormantTrack(int i)
{
    return mFormantTracks.at(i);
}

int DataStore::getFormantTrackCount() const
{
    return mFormantTracks.size();
}

void DataStore::setFormantTrackCount(int n)
{
    mFormantTracks.resize(n);
}


