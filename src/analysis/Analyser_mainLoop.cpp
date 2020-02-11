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

    // Read captured audio.
    audioLock.lock();
    audioCapture.readBlock(x);
    fs = audioCapture.getSampleRate();
    audioLock.unlock();

    // Remove DC by subtraction of the mean.
    x -= x.mean();
    
    // Apply windowing.
    applyWindow();

    // Resample audio.
    resampleAudio(2 * maximumFrequency);
    
    // Analyse spectrum if enabled.
    analyseSpectrum();

    // Get a pitch estimate.
    analysePitch();

    // Apply pre-emphasis.
    applyPreEmphasis();

    // Perform LP analysis.
    analyseLp();

    // Perform formant analysis from LP coefficients.
    analyseFormantLp();

    // Lock the tracks to prevent data race conditions.
    mutex.lock();

    // Update the tracks.
    spectra.pop_front();
    spectra.push_back(lastSpectrumFrame);
    pitchTrack.pop_front();
    pitchTrack.push_back(lastPitchFrame);
    formantTrack.pop_front();
    formantTrack.push_back(lastFormantFrame);
    
    // Smooth out the tracks.
    applyMedianFilters();
    
    // Unock the tracks.
    mutex.unlock();

    // Invoke the new-frame callback function.
    newFrameCallback();
}
