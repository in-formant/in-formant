//
// Created by rika on 16/11/2019.
//

#ifdef __linux__
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#endif

#include <chrono>
#include "Analyser.h"
#include "../lib/Pitch/Pitch.h"
#include "../lib/Signal/Filter.h"
#include "../lib/Signal/Window.h"

using namespace Eigen;

void Analyser::mainLoop()
{
    using namespace std::chrono;

    time_point t1 = steady_clock::now();

    while (running) {
        update();

        time_point t2 = steady_clock::now();

        auto dt = t2 - t1;
        if (dt < frameSpace) {
            long waitMicro = duration_cast<microseconds>(frameSpace - dt).count();

#if (_XOPEN_SOURCE >= 500) && ! (_POSIX_C_SOURCE >= 200809L) \
                   || /* Glibc since 2.19: */ _DEFAULT_SOURCE \
                   || /* Glibc versions <= 2.19: */ _BSD_SOURCE \
                   || /* Android platform */ defined(__ANDROID__)
            usleep(waitMicro);
#elif _WIN32
            HANDLE timer; 
            LARGE_INTEGER ft; 

            ft.QuadPart = -(10 * waitMicro); // Convert to 100 nanosecond interval, negative value indicates relative time

            timer = CreateWaitableTimer(nullptr, TRUE, nullptr); 
            SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0); 
            WaitForSingleObject(timer, INFINITE); 
            CloseHandle(timer); 
#else
#warning "Unknown platform, using unreliable sleep_for wait."
            std::this_thread::sleep_for(duration_cast<microseconds>(frameSpace - dt));
#endif
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
    
    fs = audioInterface->getSampleRate();
   
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
        if (v < frameCount) {
            v++;
        }
    }

    // Unlock the tracks.
    mutex.unlock();

    // Unlock the parameters.
    paramLock.unlock();
}
