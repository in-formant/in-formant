#include <QtAndroid>
#include <QAndroidJniObject>

#include "JniInstance.h"
#include "rpmalloc.h"
#include "../analysis/Analyser.h"
#include "../audio/AudioDevices.h"

std::shared_ptr<JniInstance> JniInstance::instance;

void JniInstance::createInstance(Analyser * analyser, AnalyserCanvas * canvas, PowerSpectrum * powerSpectrum)
{
    instance.reset(new JniInstance(analyser, canvas, powerSpectrum));
}

JniInstance * JniInstance::getInstance()
{
    return instance.get();
}

JniInstance::JniInstance(Analyser * analyser, AnalyserCanvas * canvas, PowerSpectrum * powerSpectrum)
    : analyser(analyser), canvas(canvas), powerSpectrum(powerSpectrum)
{
    rpm::vector<QString> pitchAlgs{
        QStringLiteral("Wavelet"),
        QStringLiteral("McLeod"),
        QStringLiteral("YIN"),
        QStringLiteral("AMDF"),
    };

    java_pitchAlgs = QAndroidJniObject("java/util/ArrayList", "(I)V", pitchAlgs.size());
    for (const auto & algName : pitchAlgs) {
        QAndroidJniObject string = QAndroidJniObject::fromString(algName);
        java_pitchAlgs.callMethod<jboolean>("add", "(Ljava/lang/Object;)Z", string.object());
    }

    rpm::vector<QString> formantAlgs{
        QStringLiteral("Linear prediction"),
        QStringLiteral("Kalman filter"),
    };

    java_formantAlgs = QAndroidJniObject("java/util/ArrayList", "(I)V", formantAlgs.size());
    for (const auto & algName : formantAlgs) {
        QAndroidJniObject string = QAndroidJniObject::fromString(algName);
        java_formantAlgs.callMethod<jboolean>("add", "(Ljava/lang/Object;)Z", string.object());
    }
}

void JniInstance::toggleAnalysis(bool running)
{
    analyser->toggle(running);
}

int JniInstance::getFftSize()
{
    return analyser->getFftSize();
}

void JniInstance::setFftSize(int fftSize)
{
    analyser->setFftSize(fftSize);
}

int JniInstance::getLpOrder()
{
    return analyser->getLinearPredictionOrder();
}

void JniInstance::setLpOrder(int lpOrder)
{
    analyser->setLinearPredictionOrder(lpOrder);
}

int JniInstance::getMaxFreq()
{
    return analyser->getMaximumFrequency();
}

void JniInstance::setMaxFreq(int maxFreq)
{
    analyser->setMaximumFrequency(maxFreq);
}

int JniInstance::getFrameLength()
{
    return analyser->getFrameLength().count();
}

void JniInstance::setFrameLength(int frameLength)
{
    analyser->setFrameLength(std::chrono::milliseconds(frameLength));
}

int JniInstance::getFrameSpace()
{
    return analyser->getFrameSpace().count();
}

void JniInstance::setFrameSpace(int frameSpace)
{
    analyser->setFrameSpace(std::chrono::milliseconds(frameSpace));
}

int JniInstance::getDuration()
{
    return analyser->getWindowSpan().count();
}

void JniInstance::setDuration(int duration)
{
    analyser->setWindowSpan(std::chrono::seconds(duration));
}

jobject JniInstance::getPitchAlgs()
{
    return java_pitchAlgs.object();
}

PitchAlg JniInstance::getPitchAlg()
{
    return analyser->getPitchAlgorithm();
}

void JniInstance::setPitchAlg(PitchAlg alg)
{
    analyser->setPitchAlgorithm(alg);
}

jobject JniInstance::getFormantAlgs()
{
    return java_formantAlgs.object();
}

FormantMethod JniInstance::getFormantAlg()
{
    return analyser->getFormantMethod();
}

void JniInstance::setFormantAlg(FormantMethod alg)
{
    analyser->setFormantMethod(alg);
}

const QColor & JniInstance::getPitchColor()
{
    return canvas->getPitchColor();
}
 
int JniInstance::getMinGain()
{
    return canvas->getMinGainSpectrum();
}

void JniInstance::setMinGain(int gain)
{
    canvas->setMinGainSpectrum(gain);
}

int JniInstance::getMaxGain()
{
    return canvas->getMaxGainSpectrum();
}

void JniInstance::setMaxGain(int gain)
{
    canvas->setMaxGainSpectrum(gain);
}

void JniInstance::setPitchColor(const QColor & color)
{
    canvas->setPitchColor(color);
}

int JniInstance::getFormantCount()
{
    return 4;
}

const QColor & JniInstance::getFormantColor(int nb)
{
    return canvas->getFormantColor(nb);
}

void JniInstance::setFormantColor(int nb, const QColor & color)
{
    canvas->setFormantColor(nb, color);
}
