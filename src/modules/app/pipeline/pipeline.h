#ifndef APP_PIPELINE_H
#define APP_PIPELINE_H

#include "../../../analysis/analysis.h"
#include "../../../nodes/nodes.h"
#include "../../audio/audio.h"

#include <unordered_map>
#include <chrono>

namespace Module::App
{
    using millis = std::chrono::milliseconds;
    using namespace std::chrono_literals;

    class Pipeline {
    public:
        Pipeline(Module::Audio::Buffer *);
        ~Pipeline();

        void initialize();

        Pipeline& setPitchSolver(Analysis::PitchSolver *);
        Pipeline& setInvglotSolver(Analysis::InvglotSolver *);
        Pipeline& setLinpredSolver(Analysis::LinpredSolver *);
        Pipeline& setFormantSolver(Analysis::FormantSolver *);

        Pipeline& setAnalysisDuration(millis);

        Pipeline& setFFTSampleRate(float);
        Pipeline& setFFTSize(int);

        Pipeline& setPreEmphasisFrequency(float);
        
        Pipeline& setPitchAndLpSpectrumSampleRate(float);
        Pipeline& setLpSpectrumLpOrder(int);

        Pipeline& setFormantSampleRate(float);
        Pipeline& setFormantLpOrder(int);

        const std::vector<std::array<float, 2>>&  getFFTSlice() const;
        const std::vector<std::array<float, 2>>&  getLpSpectrumSlice() const;
        const std::vector<float>&                 getLpSpectrumLPC() const;
        const std::vector<Analysis::FormantData>& getFormants() const;
        const float                               getPitch() const;
        const std::vector<float>&                 getSound() const;
        const std::vector<float>&                 getGlottalFlow() const;
        const std::vector<float>&                 getGlottalInstants() const;

        void processAll();

    private:
        void processStart(const std::string& nodeName);
        void processArc(const std::string& input, const std::string& output);

        void updatePrereqsForFFT();
        void updateOutputData();

        void createNodes();
        void createIOs();

        std::unordered_map<std::string, std::unique_ptr<Nodes::Node>> nodes;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Nodes::NodeIO>>> nodeIOs;

        Nodes::NodeIO **ndi, **ndo;

        std::vector<std::array<float, 2>> fftSlice;
        std::vector<std::array<float, 2>> lpSpecSlice;
        std::vector<float>                lpSpecLPC;
        std::vector<Analysis::FormantData> formants;
        float pitch;
        std::vector<float> sound;
        std::vector<float> glot;
        std::vector<float> glotInst;

        bool wasInitializedAtLeastOnce;

        millis analysisDuration;
        float captureSampleRate;

        Module::Audio::Buffer *captureBuffer;
        Analysis::PitchSolver *pitchSolver;
        Analysis::InvglotSolver *invglotSolver;
        Analysis::LinpredSolver *linpredSolver;
        Analysis::FormantSolver *formantSolver;

        float fftSampleRate;
        int fftSize;

        float preEmphasisFrequency;
        
        float secondSampleRate;
        int lpSpecLpOrder;

        float formantSampleRate;
        int formantLpOrder;
    };
}

#endif // APP_PIPELINE_H

