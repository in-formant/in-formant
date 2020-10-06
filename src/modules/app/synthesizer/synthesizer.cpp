#include "../../../analysis/analysis.h"
#include "../../../modules/math/constants.h"
#include "synthesizer.h"
#include <random>
#include <iostream>

using namespace Module::App;

Synthesizer::Synthesizer(Module::Audio::Queue *playbackQueue)
    : playbackQueue(playbackQueue),
      resampler(16'000, 48'000)
{
}

void Synthesizer::initialize()
{
    masterGain = 0.0f;
    noiseGain = 0.3f;
    glotGain = 1.0f;
    glotPitch = 170.0f;
    glotRd = 1.7f;
    formants = {{1000, 100}};
    filterShift = 1.0f;
    
    realMasterGain = 0.0f;
    realNoiseGain = 0.0f;
    realGlotGain = 0.0f;
    realGlotPitch = 170.0f;
    realGlotRd = 1.7f;
    realFilter.resize(32, 0.0f);
    realFilter[0] = 1.0f;
    realFilterShift = 1.0f;

    for (auto& zf : zfNoise)
        zf.resize(31, 0.0f);
    for (auto& zf : zfGlot)
        zf.resize(31, 0.0f);

    resampler.setOutputRate(playbackQueue->getInSampleRate());
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

void Synthesizer::setFormants(const std::vector<Analysis::FormantData>& value, float fs)
{
    formants = value;
    //resampler.setInputRate(fs);
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

    int inputLength = resampler.getRequiredInLength(requestedLength);

    auto noise = Synthesis::whiteNoise(inputLength);

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

    float glotFs = resampler.getInputRate();
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

    std::vector<float> input(inputLength);

    for (int i = 0; i < inputLength; ++i) {
        input[i] = realNoiseGain * noise[i];
    }
    for (auto& zf : zfNoise) {
        input = Synthesis::filter({1.0f}, realFilter, input, zf);
    }
    auto outputNoise = input;

    for (int i = 0; i < inputLength; ++i) {
        input[i] = realGlotGain * glot[i];
    }
    for (auto& zf : zfGlot) {
        input = Synthesis::filter({1.0f}, realFilter, input, zf);
    }
    auto outputGlot = input;

    std::vector<float> output(inputLength);
    float max = 1e-10f;
    for (int i = 0; i < inputLength; ++i) {
        output[i] = 0.5f * outputNoise[i] + outputGlot[i];
        //output[i] = realMasterGain * 0.03f * (0.5f * realNoiseGain * noise[i] + realGlotGain * glot[i]);
        if (fabsf(output[i]) > max) {
            max = fabsf(output[i]);
        }
    }

    for (int i = 0; i < inputLength; ++i) {
        output[i] = (output[i] / max) * realMasterGain * 0.5f;
    }

    output = resampler.process(output.data(), output.size());

    surplus.insert(surplus.end(), output.begin(), output.end());
}

void Synthesizer::audioCallback(float *output, int length, void *userdata)
{
    auto self = static_cast<Synthesizer *>(userdata);

    // Update the real parameters.
    self->realMasterGain = 0.1f * self->realMasterGain + 0.9f * self->masterGain;
    self->realNoiseGain  = 0.1f * self->realNoiseGain  + 0.9f * self->noiseGain;
    self->realGlotGain   = 0.1f * self->realGlotGain   + 0.9f * self->glotGain;
    self->realFilterShift = 0.2f * self->realFilterShift + 0.8f * self->filterShift;

    auto [filter, filterZeroGain] = Synthesis::frequencyShiftFilter(self->formants, self->resampler.getInputRate(), self->realFilterShift);

    self->filterZeroGain = filterZeroGain;

    for (int i = 0; i < self->realFilter.size(); ++i) {
        float filtVal = (i < filter.size()) ? filter[i] : 0.0f;
        self->realFilter[i] = 0.05f * self->realFilter[i] + 0.95f * filtVal;
    }

    // Generate the audio.
    self->generateAudio(length);

    // If we need to generate a bit more, then do so.
    while (self->surplus.size() < length) {
        self->generateAudio(256);
    }

    // Move the appropriate amount of samples from the surplus bank to the output buffer.
    std::copy(self->surplus.begin(), std::next(self->surplus.begin(), length), output);
    self->surplus.erase(self->surplus.begin(), std::next(self->surplus.begin(), length));
}
