//
// Created by rika on 16/11/2019.
//

#include <chrono>
#include "Analyser.h"
#include "../time/time_util.h"
#include "rpmalloc/rpmalloc.h"
#include "Pitch/Pitch.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"

using namespace Eigen;

void Analyser::mainLoop()
{
#ifdef Q_OS_WINDOWS
    rpmalloc_thread_initialize();
#endif

    uint64 t1 = NowInMs();

    while (running) {
        update();

        uint64 t2 = NowInMs();
        uint64 dt = t2 - t1;

        if (dt < frameSpace.count()) {
            SleepInUs(1000 * (frameSpace.count() - dt));
        }
        else {
            t1 = t2;
        }
    }
    
#ifdef Q_OS_WINDOWS
    rpmalloc_thread_finalize();
#endif
}

void Analyser::update()
{
    if (!doAnalyse) {
        return;
    }

    // Param lock.
    paramLock.lock();
    
    // Read captured audio.
    audioLock.lock();
    
    x.setZero(frameSamples);
    audioInterface->readBlock(x);

    x_fft.setZero(fftSamples);
    audioInterface->readBlock(x_fft);
    
    fs = audioInterface->getRecordSampleRate();
   
    audioLock.unlock();

    // Remove DC by subtraction of the mean.
    x -= x.mean();

    // Get a pitch estimate.
    analysePitch();
    
    // Get an Oq estimate.
    analyseOq();
    
    // Resample audio.
    resampleAudio();
    
    // Apply windowing.
    applyWindow();
  
    // Apply pre-emphasis.
    applyPreEmphasis();
 
    // Perform LP analysis.
    analyseLp();

    // Analyse spectrum.
    analyseSpectrum();

    // Perform formant analysis.
    analyseFormant();

    // Lock the tracks to prevent data race conditions.
    mutex.lock();
    
    // Update the raw tracks.
    pitchTrack.pop_front();
    pitchTrack.push_back(lastPitchFrame);
    formantTrack.pop_front();
    formantTrack.push_back(lastFormantFrame);

    // Apply postprocessing formant track correction.
    /*if (formantMethod == LP) {
        static int count = 0;
        if (++count >= (frameCount / 100)) {
            count = 0;
            trackFormants();
        }
    }*/
    
    spectra.pop_front();
    spectra.push_back(lastSpectrumFrame);

    oqTrack.pop_front();
    oqTrack.push_back(lastOqFrame);

    // Smooth out the pitch and formant tracks.
    applySmoothingFilters();

    // Set the has-new-frames flag.
    for (auto & [k, v] : nbNewFrames) {
        (void) k;

        if (v < frameCount) {
            v++;
        }
    }

    // Unlock the tracks.
    mutex.unlock();

    // Unlock the parameters.
    paramLock.unlock();
}
