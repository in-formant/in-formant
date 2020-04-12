//
// Created by rika on 16/11/2019.
//

#include <chrono>
#include "Analyser.h"
#include "../lib/Pitch/Pitch.h"
#include "../lib/Signal/Filter.h"
#include "../lib/Signal/Window.h"

using namespace Eigen;

void Analyser::mainLoop()
{
    using namespace std::chrono;

    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    while (running) {
        update();

        high_resolution_clock::time_point t2 = high_resolution_clock::now();

        auto dt = t2 - t1;
        if (dt < frameSpace) {
            std::this_thread::sleep_for(frameSpace - dt);
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
    audioCapture->readBlock(x);

    x_fft.setZero(fftSamples);
    audioCapture->readBlock(x_fft);
    
    fs = audioCapture->getSampleRate();
   
    audioLock.unlock();

    // Remove DC by subtraction of the mean.
    x -= x.mean();

    // Get a pitch estimate.
    analysePitch();
    
    // Get an Oq estimate.
    analyseOq();
    
    // Resample audio.
    resampleAudio(2 * maximumFrequency);
    
    // Apply windowing.
    applyWindow();
  
    // Apply pre-emphasis.
    applyPreEmphasis();
    
    // Analyse spectrum.
    analyseSpectrum();

    // Perform LP analysis.
    analyseLp();

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
    if (formantMethod == LP) {
        static int count = 0;
        if (++count >= (frameCount / 100)) {
            count = 0;
            trackFormants();
        }
    }
    
    spectra.pop_front();
    spectra.push_back(lastSpectrumFrame);

    oqTrack.pop_front();
    oqTrack.push_back(lastOqFrame);

    // Smooth out the pitch and formant tracks.
    applySmoothingFilters();

    // Set the has-new-frames flag.
    for (auto & [k, v] : nbNewFrames) {
        v++;
    }

    // Unlock the tracks.
    mutex.unlock();

    // Unlock the parameters.
    paramLock.unlock();
}
