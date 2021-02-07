#include "../../../analysis/analysis.h"
#include "synthesizer.h"
#include <random>
#include <iostream>

using namespace Module::App;

Synthesizer::Synthesizer(Module::Audio::Queue *playbackQueue)
    : playbackQueue(playbackQueue),
      resampler(new Module::Audio::Resampler(32'000, 48'000))
{
    playbackQueue->setCallback(App::Synthesizer::audioCallback);
}

Synthesizer::~Synthesizer()
{
    delete resampler;
}

void Synthesizer::initialize()
{
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

    resampler->setOutputRate(playbackQueue->getInSampleRate());
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

    int inputLength = resampler->getRequiredInLength(requestedLength);

    auto noise = Synthesis::aspirateNoise(inputLength);

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

    double glotFs = resampler->getInputRate();
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
            Rds[i]     = expFact * Rds[i - 1]     + (1 - expFact) * glotRd;
            tcs[i]     = expFact * tcs[i - 1]     + (1 - expFact) * glotTc;
        }
        realGlotPitch = pitches.back();
        realGlotRd = Rds.back();
        realGlotTc = tcs.back();
    }

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
        lpsos = Analysis::butterworthLowpass(8, 16000, glotFs);
    }

    static rpm::vector<rpm::vector<double>> lpglotmem(20, rpm::vector<double>(4, 0.0));
    glot = Synthesis::sosfilter(lpsos, glot, lpglotmem);

    rpm::vector<double> input(inputLength);

    auto outputNoise = Synthesis::sosfilter(realFilter, noise, zfNoise);
    double outputNoiseMax = 0.0;
    for (int i = 0; i < inputLength; ++i) {
        if (fabs(outputNoise[i]) > outputNoiseMax) {
            outputNoiseMax = fabs(outputNoise[i]);
        }
    }

    auto outputGlot = Synthesis::sosfilter(realFilter, glot, zfGlot);
    double outputGlotMax = 0.0;
    for (int i = 0; i < inputLength; ++i) {
        if (fabs(outputGlot[i]) > outputGlotMax) {
            outputGlotMax = fabs(outputGlot[i]);
        }
    }

    rpm::vector<double> output(inputLength);
    for (int i = 0; i < inputLength; ++i) {
        if (voiced) {
            output[i] = 0.5 * realNoiseGain * outputNoise[i] / outputNoiseMax + realGlotGain * outputGlot[i] / outputGlotMax;
        }
        else {
            output[i] = 0.7 * realNoiseGain * outputNoise[i] / outputNoiseMax;
        }
        //output[i] = realMasterGain * 0.03 * (0.5 * realNoiseGain * noise[i] + realGlotGain * glot[i]);
    }

    for (int i = 0; i < inputLength; ++i) {
        output[i] = output[i] * realMasterGain * 0.7;
    }

    output = resampler->process(output.data(), output.size());

    surplus.insert(surplus.end(), output.begin(), output.end());
}

void Synthesizer::audioCallback(double *output, int length, void *userdata)
{
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
            self->realFormants[i].frequency = 0.4 * self->realFormants[i].frequency + 0.6 * self->formants[i].frequency;
            self->realFormants[i].bandwidth = 0.2 * self->realFormants[i].bandwidth + 0.8 * self->formants[i].bandwidth;
        }
    }

    self->realFilter = Synthesis::frequencyShiftFilter(self->realFormants, self->resampler->getInputRate(), self->realFilterShift);

    // Generate the audio.
    self->generateAudio(length);

    // If we need to generate a bit more, then do so.
    while (self->surplus.size() < length) {
        self->generateAudio(256);
    }

    // Move the appropriate amount of samples from the surplus bank to the output buffer.
    if (length > 0) {
        std::copy(self->surplus.begin(), std::next(self->surplus.begin(), length), output);
        self->surplus.erase(self->surplus.begin(), std::next(self->surplus.begin(), length));
    }
}
