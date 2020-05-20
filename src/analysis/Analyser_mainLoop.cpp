//
// Created by rika on 16/11/2019.
//

#include <chrono>
#include "Analyser.h"
#include "../time/time_util.h"
#include "Pitch/Pitch.h"
#include "Signal/Filter.h"
#include "Signal/Window.h"

using namespace Eigen;

void Analyser::mainLoop()
{
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
   
    if (formantMethod == DeepFormants) {
        analyseDeepFormants();
    }
  
    // Apply pre-emphasis.
    applyPreEmphasis();

    // Get an Oq estimate.
    //analyseOq();
    
    // Do inverse filtering and export signals.
    mutex.lock();
    applyInverseFilter();
    mutex.unlock();

    // Resample audio.
    resampleAudio(); 
    
    // Apply windowing. 
    applyWindow();

    // Perform LP analysis.
    analyseLp();

    // Analyse spectrum.
    analyseSpectrum();

    // Perform formant analysis. 
    if (formantMethod != DeepFormants) {
        analyseFormant();
    }

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
    if (nbNewFrames < frameCount - 1) {
        nbNewFrames++;
    }

    // Unlock the tracks.
    mutex.unlock();

    // Unlock the parameters.
    paramLock.unlock();
}
