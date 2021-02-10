#include "datastore.h"

using namespace Main;

DataStore::DataStore()
    : mTrackLength(0),
      mCatchupCount(1),
      mTime(0)
{
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

TimeTrack<rpm::vector<double>>& DataStore::getSoundTrack()
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

rpm::vector<SpectrogramCoefs>& DataStore::getSpectrogramCoefs()
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


