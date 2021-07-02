#ifndef WITHOUT_SYNTH

#include "../../../analysis/analysis.h"
#include "../../../context/timings.h"
#include "synthesizer.h"
#include <random>
#include <iostream>

using namespace Module::App;

Synthesizer::Synthesizer(Module::Audio::Queue *playbackQueue)
    : playbackQueue(playbackQueue),
      resampler(48'000)
{
    playbackQueue->setCallback(App::Synthesizer::audioCallback);
    
    masterGain = 0.0;
    noiseGain = 0.3;
    glotGain = 1.0;
    glotPitch = 170.0;
    glotRd = 1.7;
    glotTc = 1.0;
    formants = {{1000, 100}};
    filterShift = 1.0;
    voiced = false;
    
    realMasterGain = 0.0;
    realNoiseGain = 0.0;
    realGlotGain = 0.0;
    realGlotPitch = 170.0;
    realGlotRd = 1.7;
    realGlotTc = 1.0;
    realFilterShift = 1.0;
    realFilter.clear();

    zfNoise.resize(20, rpm::vector<double>(4, 0.0));
    zfGlot.resize(20, rpm::vector<double>(4, 0.0));

    resampler.setOutputRate(playbackQueue->getInSampleRate());
}

void Synthesizer::setMasterGain(double value)
{
    masterGain = value;
}

void Synthesizer::setNoiseGain(double value)
{
    noiseGain = value;
}

void Synthesizer::setGlotGain(double value)
{
    glotGain = value;
}

void Synthesizer::setGlotPitch(double value)
{
    glotPitch = value;
}

void Synthesizer::setGlotRd(double value)
{
    glotRd = value;
}

void Synthesizer::setGlotTc(double value)
{
    glotTc = value;
}

void Synthesizer::setFormants(const rpm::vector<Analysis::FormantData>& value)
{
    formants = value;
}

void Synthesizer::setFilterShift(double value)
{
    filterShift = value;
}

void Synthesizer::setVoiced(bool value)
{
    voiced = value;
}

double Synthesizer::getMasterGain() const
{
    return realMasterGain;
}

double Synthesizer::getNoiseGain() const
{
    return realNoiseGain;
}

double Synthesizer::getGlotGain() const
{
    return realGlotGain;
}

double Synthesizer::getGlotPitch() const
{
    return glotPitch;
}

double Synthesizer::getGlotRd() const
{
    return glotRd;
}

double Synthesizer::getGlotTc() const
{
    return glotTc;
}

double Synthesizer::getFilterShift() const
{
    return filterShift;
}

bool Synthesizer::isVoiced() const
{
    return voiced;
}

rpm::vector<std::array<double, 6>> Synthesizer::getFilterCopy(double *fs) const
{
    *fs = resampler.getInputRate();
    
    realFilterMutex.lock();
    auto filterCopy = realFilter;
    realFilterMutex.unlock();

    return filterCopy;
}

rpm::vector<double> Synthesizer::getSourceCopy(double fs, double durationInMs) const
{
#ifdef __WIN32
#   define rd rand
#else
    static std::random_device rd;
#endif

#if SIZEOF_VOID_P == 4
    static std::mt19937 gen(rd());
#else
    static std::mt19937_64 gen(rd());
#endif

    static std::normal_distribution<> dis(0.0, 0.05);

    int length = std::round((durationInMs * fs) / 1000);

    auto source = Synthesis::aspirateNoise(length);

    rpm::vector<double> pitches(length);
    rpm::vector<double> Rds(length);
    rpm::vector<double> tcs(length);
    
    constexpr double expFact = 0.997;
    pitches[0] = realGlotPitch;
    Rds[0] = realGlotRd;
    tcs[0] = realGlotTc;
    for (int i = 1; i < length; ++i) {
        pitches[i] = expFact * pitches[i - 1] + (1 - expFact) * (glotPitch * (1 + dis(gen)));
        Rds[i]     = expFact * Rds[i - 1]     + (1 - expFact) * (glotRd + dis(gen));
        tcs[i]     = expFact * tcs[i - 1]     + (1 - expFact) * (glotTc + dis(gen));
    }

    rpm::vector<double> glot;
    while (glot.size() < length) {
        auto frame = Synthesis::lfGenFrame(pitches[glot.size()], fs, Rds[glot.size()], tcs[glot.size()]);
        glot.insert(glot.end(), frame.begin(), frame.end());
    }

    static rpm::vector<std::array<double, 6>> lpsos;
    static int lastSampleRate = 0;
    if (lpsos.empty() || lastSampleRate != fs) {
        lastSampleRate = fs;
        lpsos = Analysis::butterworthLowpass(10, fs * 0.49, fs);
    }

    glot = Analysis::sosfilter(lpsos, glot);

    for (int i = 0; i < length; ++i) {
        source[i] = 0.01 * realNoiseGain * source[i] + realGlotGain * glot[i];
    }
    return source;
}

void Synthesizer::generateAudio(int requestedLength)
{
#ifdef __WIN32
#   define rd rand
#else
    static std::random_device rd;
#endif

#if SIZEOF_VOID_P == 4
    static std::mt19937 gen(rd());
#else
    static std::mt19937_64 gen(rd());
#endif

    static std::normal_distribution<> dis(0.0, 0.05);

    int inputLength = resampler.getRequiredInLength(requestedLength);

    // Still update the parameters regardless of whether we're generating noise/glot or not.
    rpm::vector<double> pitches(inputLength);
    rpm::vector<double> Rds(inputLength);
    rpm::vector<double> tcs(inputLength);
    if (inputLength > 0) {
        constexpr double expFact = 0.997;
        pitches[0] = realGlotPitch;
        Rds[0] = realGlotRd;
        tcs[0] = realGlotTc;
        for (int i = 1; i < inputLength; ++i) {
            pitches[i] = expFact * pitches[i - 1] + (1 - expFact) * (glotPitch * (1 + dis(gen)));
            Rds[i]     = expFact * Rds[i - 1]     + (1 - expFact) * (glotRd + dis(gen));
            tcs[i]     = expFact * tcs[i - 1]     + (1 - expFact) * (glotTc + dis(gen));
        }
        realGlotPitch = pitches.back();
        realGlotRd = Rds.back();
        realGlotTc = tcs.back();
    }

    rpm::vector<double> output(inputLength, 0.0);

    if (realMasterGain > 1e-7) {

        // GENERATE NOISE

        rpm::vector<double> outputNoise(inputLength, 0.0);
        double outputNoiseMax = 1e-13;

        /*if (realNoiseGain > 1e-7)*/ {
            auto noise = Synthesis::aspirateNoise(inputLength);

            outputNoise = Synthesis::sosfilter(realFilter, noise, zfNoise);
            for (int i = 0; i < inputLength; ++i) {
                if (fabs(outputNoise[i]) > outputNoiseMax) {
                    outputNoiseMax = fabs(outputNoise[i]);
                }
            }
        }
        
        // GENERATE GLOT

        rpm::vector<double> outputGlot(inputLength, 0.0);
        double outputGlotMax = 1e-13;

        if (realGlotGain > 1e-7) {
            rpm::vector<double> glot;
            glot.reserve(inputLength);
        
            if (glotSurplus.size() > 0) {
                if (glotSurplus.size() <= inputLength) {
                    glot.insert(glot.begin(), glotSurplus.begin(), glotSurplus.end());
                    glotSurplus.clear();
                }
                else {
                    glot.insert(glot.begin(), glotSurplus.begin(), std::next(glotSurplus.begin(), inputLength));
                    glotSurplus.erase(glotSurplus.begin(), std::next(glotSurplus.begin(), inputLength));
                }
            }

            double glotFs = resampler.getInputRate();

            while (glot.size() < inputLength) {
                auto frame = Synthesis::lfGenFrame(pitches[glot.size()], glotFs, Rds[glot.size()], tcs[glot.size()]);

                glot.insert(glot.end(), frame.begin(), frame.end());
            }

            if (glot.size() > inputLength) {
                if (inputLength > 0) {
                    glotSurplus.insert(glotSurplus.end(), std::next(glot.begin(), inputLength), glot.end());
                }
                glot.resize(inputLength);
            }

            // Anti-alias glot.
            static rpm::vector<std::array<double, 6>> lpsos;
            static int lastSampleRate = 0;

            if (lpsos.empty() || lastSampleRate != glotFs) {
                lastSampleRate = glotFs;
                lpsos = Analysis::butterworthLowpass(10, glotFs * 0.49, glotFs);
            }

            static rpm::vector<rpm::vector<double>> lpglotmem(20, rpm::vector<double>(4, 0.0));
            glot = Synthesis::sosfilter(lpsos, glot, lpglotmem);

            outputGlot = Synthesis::sosfilter(realFilter, glot, zfGlot);
            for (int i = 0; i < inputLength; ++i) {
                if (fabs(outputGlot[i]) > outputGlotMax) {
                    outputGlotMax = fabs(outputGlot[i]);
                }
            }
        }

        for (int i = 0; i < inputLength; ++i) {
            output[i] = realNoiseGain * 0.2 * outputNoise[i] / outputNoiseMax;
            if (voiced)
                output[i] += realGlotGain * outputGlot[i] / outputGlotMax;
            output[i] *= 0.8 * realMasterGain;
        }
    }

    output = resampler.process(output.data(), (int) output.size());

    surplus.insert(surplus.end(), output.begin(), output.end());
}

void Synthesizer::audioCallback(double *output, int length, void *userdata)
{
    timer_guard timer(timings::synth);

    auto self = static_cast<Synthesizer *>(userdata);

    // Update the real parameters.
    self->realMasterGain = 0.1 * self->realMasterGain + 0.9 * self->masterGain;
    self->realNoiseGain  = 0.1 * self->realNoiseGain  + 0.9 * self->noiseGain;
    self->realGlotGain   = 0.1 * self->realGlotGain   + 0.9 * self->glotGain;
    self->realFilterShift = 0.2 * self->realFilterShift + 0.8 * self->filterShift;

    static double formantCount = 0;

    if (formantCount == 0) {
        formantCount = self->formants.size();
    }
    else {
        formantCount = 0.4 * formantCount + 0.6 * self->formants.size();
    }

    self->realFormants.resize(std::round(formantCount));

    for (int i = 0; i < self->formants.size(); ++i) {
        if (i < std::round(formantCount)) {
            self->realFormants[i].frequency = 0.9 * self->realFormants[i].frequency + 0.1 * self->formants[i].frequency;
            self->realFormants[i].bandwidth = 0.2 * self->realFormants[i].bandwidth + 0.8 * self->formants[i].bandwidth;
        }
    }

    self->realFilterMutex.lock();
    self->realFilter = Synthesis::frequencyShiftFilter(self->realFormants, self->resampler.getInputRate(), self->realFilterShift);
    self->realFilterMutex.unlock();

    // Generate the audio by chunks of fixed size.
    while (self->surplus.size() < length) {
        self->generateAudio(4096);
    }

    // Move the appropriate amount of samples from the surplus bank to the output buffer.
    if (length > 0) {
        std::copy(self->surplus.begin(), std::next(self->surplus.begin(), length), output);
        self->surplus.erase(self->surplus.begin(), std::next(self->surplus.begin(), length));
    }
}

#endif // !WITHOUT_SYNTH
