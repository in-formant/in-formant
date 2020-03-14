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

    // Get a pitch estimate.
    analysePitch();

    // Resample audio.
    resampleAudio(2 * maximumFrequency);
  
    // Apply pre-emphasis.
    applyPreEmphasis();
    
    // Analyse spectrum.
    analyseSpectrum();

    // Apply windowing.
    applyWindow();

    // Perform LP analysis.
    analyseLp();

    // Perform formant analysis from LP coefficients.
    analyseFormantLp();

    // Lock the tracks to prevent data race conditions.
    mutex.lock();

    // Update the raw tracks.
    pitchTrack.pop_front();
    pitchTrack.push_back(lastPitchFrame);
    formantTrack.pop_front();
    formantTrack.push_back(lastFormantFrame);

    // Apply postprocessing formant track correction.
    formantTrack = Formant::track(
        formantTrack,
        3,
        550,
        1650,
        2750,
        3850,
        4950,
        1.0,
        1.0,
        1.0
    );

    spectra.pop_front();
    spectra.push_back(lastSpectrumFrame);

    // Smooth out the pitch and formant tracks.
    applySmoothingFilters();
    
    // Unlock the tracks.
    mutex.unlock();

    // Invoke the new-frame callback function.
    newFrameCallback();
}
