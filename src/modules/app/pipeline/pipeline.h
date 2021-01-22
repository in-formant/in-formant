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

        Pipeline& setFFTSampleRate(double);
        Pipeline& setFFTSize(int);

        Pipeline& setPreEmphasisFrequency(double);
        
        Pipeline& setPitchAndLpSpectrumSampleRate(double);
        Pipeline& setLpSpectrumLpOrder(int);

        Pipeline& setFormantSampleRate(double);
        Pipeline& setFormantLpOrder(int);

        const std::vector<std::array<double, 2>>&  getFFTSlice() const;
        const std::vector<std::array<double, 2>>&  getLpSpectrumSlice() const;
        const std::vector<double>&                 getLpSpectrumLPC() const;
        const std::vector<Analysis::FormantData>& getFormants() const;
        double                                     getPitch() const;
        const std::vector<double>&                 getSound() const;
        const std::vector<double>&                 getGlottalFlow() const;
        const std::vector<double>&                 getGlottalInstants() const;

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

        std::vector<std::array<double, 2>> fftSlice;
        std::vector<std::array<double, 2>> lpSpecSlice;
        std::vector<double>                lpSpecLPC;
        std::vector<Analysis::FormantData> formants;
        double pitch;
        std::vector<double> sound;
        std::vector<double> glot;
        std::vector<double> glotInst;

        bool wasInitializedAtLeastOnce;

        millis analysisDuration;
        double captureSampleRate;

        Module::Audio::Buffer *captureBuffer;
        Analysis::PitchSolver *pitchSolver;
        Analysis::InvglotSolver *invglotSolver;
        Analysis::LinpredSolver *linpredSolver;
        Analysis::FormantSolver *formantSolver;

        double fftSampleRate;
        int fftSize;

        double preEmphasisFrequency;
        
        double secondSampleRate;
        int lpSpecLpOrder;

        double formantSampleRate;
        int formantLpOrder;
    };
}

#endif // APP_PIPELINE_H

