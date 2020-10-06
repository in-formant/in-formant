#include "../../../analysis/analysis.h"
#include "../../../modules/math/constants.h"
#include "synthesizer.h"
#include <random>
#include <iostream>

using namespace Module::App;

Synthesizer::Synthesizer(Module::Audio::Queue *playbackQueue)
    : playbackQueue(playbackQueue),
      noiseResampler(12'000, 16'000),
      outputResampler(16'000, 48'000)
{
}

void Synthesizer::initialize()
{
    masterGain = 0.0f;
    noiseGain = 0.3f;
    glotGain = 1.0f;
    glotPitch = 170.0f;
    glotRd = 1.7f;
    filter = { 1.0f };
    filterShift = 1.0f;
    
    realMasterGain = 0.0f;
    realNoiseGain = 0.0f;
    realGlotGain = 0.0f;
    realGlotPitch = 170.0f;
    realGlotRd = 1.7f;
    realFilter.resize(100, 0.0f);
    realFilter[0] = 1.0f;
    realFilterShift = 1.0f;

    for (auto& mem : filterMemoryNoise)
        mem.resize(100, 0.0f);
    for (auto& mem : filterMemoryGlot)
        mem.resize(100, 0.0f);
}

void Synthesizer::setMasterGain(float value)
{
    masterGain = value;
}

void Synthesizer::setNoiseGain(float value)
{
    noiseGain = value;
}

void Synthesizer::setGlotGain(float value)
{
    glotGain = value;
}

void Synthesizer::setGlotPitch(float value)
{
    glotPitch = value;
}

void Synthesizer::setGlotRd(float value)
{
    glotRd = value;
}

void Synthesizer::setFilter(const std::vector<float>& filt, float sampleRate)
{
    filter.resize(filt.size() + 1);
    filter[0] = 1.0f;
    std::copy(filt.begin(), filt.end(), std::next(filter.begin()));
    noiseResampler.setOutputRate(sampleRate);
    outputResampler.setInputRate(sampleRate);
}

void Synthesizer::setFilterShift(float value)
{
    filterShift = value;
}

float Synthesizer::getMasterGain() const
{
    return realMasterGain;
}

float Synthesizer::getNoiseGain() const
{
    return realNoiseGain;
}

float Synthesizer::getGlotGain() const
{
    return realGlotGain;
}

float Synthesizer::getGlotPitch() const
{
    return glotPitch;
}

float Synthesizer::getGlotRd() const
{
    return glotRd;
}

float Synthesizer::getFilterShift() const
{
    return filterShift;
}

void Synthesizer::generateAudio(int requestedLength)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> dis(0.0f, 0.02f);

    int inputLength = outputResampler.getRequiredInLength(requestedLength);
    int noiseLength = noiseResampler.getRequiredInLength(inputLength);
    int outputLength;
    
    auto noise = Synthesis::whiteNoise(noiseLength);

    noise = noiseResampler.process(noise.data(), noise.size());
    inputLength = noise.size();

    std::vector<float> glot;
    glot.reserve(inputLength);
   
    if (glotSurplus.size() > 0) {
        if (glotSurplus.size() <= inputLength) {
            glot.insert(glot.begin(), glotSurplus.begin(), glotSurplus.end());
            glotSurplus.erase(glotSurplus.begin(), glotSurplus.end());
        }
        else {
            glot.insert(glot.begin(), glotSurplus.begin(), std::next(glotSurplus.begin(), inputLength));
            glotSurplus.erase(glotSurplus.begin(), std::next(glotSurplus.begin(), inputLength));
        }
    }

    float glotFs = outputResampler.getInputRate();
    std::vector<float> pitches(inputLength);
    std::vector<float> Rds(inputLength);
    if (inputLength > 0) {
        constexpr float expFact = 0.8f;
        pitches[0] = realGlotPitch;
        Rds[0] = realGlotRd;
        for (int i = 1; i < inputLength; ++i) {
            pitches[i] = expFact * pitches[i - 1] + (1 - expFact) * (glotPitch * (1 + dis(gen)));
            Rds[i]     = expFact * Rds[i - 1]     + (1 - expFact) * glotRd;
        }
        realGlotPitch = pitches.back();
        realGlotRd = Rds.back();
    }

    while (glot.size() < inputLength) {
        auto frame = Synthesis::lfGenFrame(pitches[glot.size()], glotFs, Rds[glot.size()]);
        glot.insert(glot.end(), frame.begin(), frame.end());
    }

    if (glot.size() > inputLength) {
        glotSurplus.insert(glotSurplus.end(), std::next(glot.begin(), inputLength), glot.end());
        glot.resize(inputLength);
    }

    // Anti-alias glot.
    for (int i = inputLength - 1; i >= 1; --i) {
        glot[i] -= expf(-2.0f * M_PI * (glotFs / 2 - 100.0f) / glotFs) * glot[i - 1];
    }

    // TODO: NEED TO WRITE AN ACTUAL FILTER SHIFT ROUTINE
    auto shiftedFilter = Synthesis::frequencyShiftFilter(realFilter, glotFs, realFilterShift);

    std::vector<float> input(inputLength);

    for (int i = 0; i < inputLength; ++i) {
        input[i] = realNoiseGain * noise[i];
    }
    for (auto& mem : filterMemoryNoise) {
        input = Synthesis::filter({1.0f}, shiftedFilter, input, mem);
    }
    auto outputNoise = input;

    for (int i = 0; i < inputLength; ++i) {
        input[i] = realGlotGain * glot[i];
    }
    for (auto& mem : filterMemoryGlot) {
        input = Synthesis::filter({1.0f}, shiftedFilter, input, mem);
    }
    auto outputGlot = input;

    std::vector<float> output(inputLength);
    for (int i = 0; i < inputLength; ++i) {
        output[i] = 0.03f * (0.5f * outputNoise[i] + outputGlot[i]);
    }

    output = outputResampler.process(output.data(), output.size());
    outputLength = output.size();

    for (int i = 0; i < outputLength; ++i) {
        output[i] *= realMasterGain;
    }

    surplus.insert(surplus.end(), output.begin(), output.end());
}

void Synthesizer::audioCallback(float *output, int length, void *userdata)
{
    auto self = static_cast<Synthesizer *>(userdata);

    // Update the real parameters.
    self->realMasterGain = 0.1f * self->realMasterGain + 0.9f * self->masterGain;
    self->realNoiseGain  = 0.1f * self->realNoiseGain  + 0.9f * self->noiseGain;
    self->realGlotGain   = 0.1f * self->realGlotGain   + 0.9f * self->glotGain;

    for (int i = 0; i < self->realFilter.size(); ++i) {
        float filtVal = (i < self->filter.size()) ? self->filter[i] : 0.0f;
        self->realFilter[i] = 0.05f * self->realFilter[i] + 0.95f * filtVal;
    }

    self->realFilterShift = 0.2f * self->realFilterShift + 0.8f * self->filterShift;

    // Generate the audio.
    self->generateAudio(length);

    // If we need to generate a bit more, then do so.
    while (self->surplus.size() < length) {
        self->generateAudio(512);
    }

    // Move the appropriate amount of samples from the surplus bank to the output buffer.
    std::copy(self->surplus.begin(), std::next(self->surplus.begin(), length), output);
    self->surplus.erase(self->surplus.begin(), std::next(self->surplus.begin(), length));
}
