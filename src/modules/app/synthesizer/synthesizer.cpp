#include "synthesizer.h"

using namespace Module::App;

Synthesizer::Synthesizer(Module::Audio::Queue *playbackQueue)
    : playbackQueue(playbackQueue),
      noiseResampler(10'000, 16'000),
      outputResampler(16'000, 48'000)
{
}

void Synthesizer::initialize()
{
    masterGain = 0.0f;
    noiseGain = 0.3f;
    glotGain = 1.0f;
    filter = { 1.0f };
    
    realMasterGain = 0.0f;
    realNoiseGain = 0.0f;
    realGlotGain = 0.0f;
    realFilter.resize(40, 0.0f);
    realFilter[0] = 1.0f;

    filterMemory.resize(40, 0.0f);
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

void Synthesizer::setFilter(const std::vector<float>& filt, float sampleRate)
{
    filter = filt;
    filter.insert(filter.begin(), 1.0f);
    noiseResampler.setOutputRate(sampleRate);
    outputResampler.setInputRate(sampleRate);
}

void Synthesizer::generateAudio(int requestedLength)
{
    int noiseLength = (requestedLength * noiseResampler.getInputRate())
                                / outputResampler.getOutputRate();
    int inputLength;
    int outputLength;
    
    auto noise = Synthesis::whiteNoise(noiseLength);

    noise = noiseResampler.process(noise.data(), noise.size());
    inputLength = noise.size();

    //auto glot = Synthesis::generateGlottalSource();

    std::vector<float> input(inputLength);
    for (int i = 0; i < inputLength; ++i) {
        input[i] = realNoiseGain * noise[i]; // + realGlotGain * glot[i];
    }

    auto output = Synthesis::filter({0.025f}, realFilter, input, filterMemory);

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
    self->realMasterGain = 0.8f * self->realMasterGain + 0.2f * self->masterGain;
    self->realNoiseGain  = 0.8f * self->realNoiseGain  + 0.2f * self->noiseGain;
    self->realGlotGain   = 0.8f * self->realGlotGain   + 0.2f * self->glotGain;

    for (int i = 0; i < self->realFilter.size(); ++i) {
        float filtVal = (i < self->filter.size()) ? self->filter[i] : 0.0f;
        self->realFilter[i] = 0.4f * self->realFilter[i] + 0.6f * filtVal;
    }

    // Generate the audio.
    self->generateAudio(length);

    // If we need to generate a bit more, then do so.
    while (self->surplus.size() < length) {
        self->generateAudio(32);
    }

    // Move the appropriate amount of samples from the surplus bank to the output buffer.
    std::copy(self->surplus.begin(), std::next(self->surplus.begin(), length), output);
    self->surplus.erase(self->surplus.begin(), std::next(self->surplus.begin(), length));
}
