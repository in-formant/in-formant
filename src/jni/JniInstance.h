#ifndef JNI_INSTANCE_H
#define JNI_INSTANCE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <QColor>
#include <QString>
#include <QAndroidJniObject>
#include <jni.h>
#include "../audio/miniaudio.h"

#include "../analysis/Analyser.h"
#include "../gui/AnalyserCanvas.h"
#include "../gui/PowerSpectrum.h"

class JniInstance {
public:
    static void createInstance(Analyser * analyser, AnalyserCanvas * canvas, PowerSpectrum * powerSpectrum);
    static JniInstance * getInstance();
   
    void toggleAnalysis(bool running);

    int getFftSize();
    void setFftSize(int fftSize);

    int getLpOrder();
    void setLpOrder(int lpOrder);

    int getMaxFreq();
    void setMaxFreq(int maxFreq);

    int getFrameLength();
    void setFrameLength(int frameLength);

    int getFrameSpace();
    void setFrameSpace(int frameSpace);

    int getDuration();
    void setDuration(int duration);

    jobject getPitchAlgs();
    PitchAlg getPitchAlg();
    void setPitchAlg(PitchAlg alg);

    jobject getFormantAlgs();
    FormantMethod getFormantAlg();
    void setFormantAlg(FormantMethod alg);

    int getMinGain();
    void setMinGain(int gain);

    int getMaxGain();
    void setMaxGain(int gain);

    const QColor & getPitchColor();
    void setPitchColor(const QColor & color);

    int getFormantCount();
    const QColor & getFormantColor(int nb);
    void setFormantColor(int nb, const QColor & color);

private:
    JniInstance(Analyser * analyser, AnalyserCanvas * canvas, PowerSpectrum * powerSpectrum);
    
    Analyser * analyser;
    AnalyserCanvas * canvas;
    PowerSpectrum * powerSpectrum;

    std::mutex mut;
    
    static std::shared_ptr<JniInstance> instance;

    QAndroidJniObject java_pitchAlgs;
    QAndroidJniObject java_formantAlgs;
};

#endif // JNI_INSTANCE_H
