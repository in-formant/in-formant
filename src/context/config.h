#ifndef MAIN_CONTEXT_CONFIG_H
#define MAIN_CONTEXT_CONFIG_H

#include <QObject>
#include <filesystem>
#include <fstream>
#include <toml++/toml.h>

#include "solvermakers.h"
#include "../modules/audio/base/base.h"
#include "../modules/renderer/base/parameters.h"

namespace fs = std::filesystem;

namespace Main {

    using Module::Renderer::FrequencyScale;

    fs::path getConfigPath();
    toml::table getConfigTable();
   
    class Config : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool viewShowSpectrogram READ getViewShowSpectrogram WRITE setViewShowSpectrogram NOTIFY configChanged)
        Q_PROPERTY(bool viewShowPitch       READ getViewShowPitch       WRITE setViewShowPitch       NOTIFY configChanged)
        Q_PROPERTY(bool viewShowFormants    READ getViewShowFormants    WRITE setViewShowFormants    NOTIFY configChanged)

    signals:
        void configChanged();

    public:
        Config();
        virtual ~Config();

        Module::Audio::Backend getAudioBackend();

        PitchAlgorithm getPitchAlgorithm();
        LinpredAlgorithm getLinpredAlgorithm();
        FormantAlgorithm getFormantAlgorithm();
        InvglotAlgorithm getInvglotAlgorithm();

        int getViewFontSize();
        int getViewMinFrequency();
        int getViewMaxFrequency();
        int getViewFFTSize();
        int getViewMinGain();
        int getViewMaxGain();
        FrequencyScale getViewFrequencyScale();
        int getViewFormantCount();
        std::array<double, 3> getViewFormantColor(int i);

        bool getViewShowSpectrogram();
        void setViewShowSpectrogram(bool b);

        bool getViewShowPitch();
        void setViewShowPitch(bool b);

        bool getViewShowFormants();
        void setViewShowFormants(bool b);

        int getAnalysisMaxFrequency();
        int getAnalysisLpOffset();
        int getAnalysisPitchSampleRate();

    private:
        toml::table mTbl;
    };

}

#endif // MAIN_CONTEXT_CONFIG_H
