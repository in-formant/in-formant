#include <vector>
#include "JniInstance.h"

#define declare_binding(ret, name) extern "C" JNIEXPORT ret JNICALL Java_fr_cloyunhee_speechanalysis_JniBridge_##name (JNIEnv * env, jobject obj)

#define declare_binding2(name, args...) extern "C" JNIEXPORT void JNICALL Java_fr_cloyunhee_speechanalysis_JniBridge_##name (JNIEnv * env, jobject obj, args)

#define declare_binding3(ret, name, args...) extern "C" JNIEXPORT ret JNICALL Java_fr_cloyunhee_speechanalysis_JniBridge_##name (JNIEnv * env, jobject obj, args)

// Version info

static QAndroidJniObject versionStringObject = QAndroidJniObject::fromString(VERSION_STRING);

declare_binding(jstring, getVersionString) {
    return versionStringObject.object<jstring>();
}

declare_binding(jint, getVersionCode) {
    return VERSION_CODE;
}

// Toggle analysis

declare_binding2(toggleAnalysis, jboolean running) {
    JniInstance::getInstance()->toggleAnalysis(running);
}

// FFT size

declare_binding(jint, getFftSize) {
    return JniInstance::getInstance()->getFftSize();
}

declare_binding2(setFftSize, jint fftSize) {
    JniInstance::getInstance()->setFftSize(fftSize);
}

// Linear prediction order

declare_binding(jint, getLpOrder) {
    return JniInstance::getInstance()->getLpOrder();
}

declare_binding2(setLpOrder, jint lpOrder) {
    JniInstance::getInstance()->setLpOrder(lpOrder);
}

// Maximum formant frequency

declare_binding(jint, getMaxFreq) {
    return JniInstance::getInstance()->getMaxFreq();
}

declare_binding2(setMaxFreq, jint maxFreq) {
    JniInstance::getInstance()->setMaxFreq(maxFreq);
}

// Frame length

declare_binding(jint, getFrameLength) {
    return JniInstance::getInstance()->getFrameLength();
}

declare_binding2(setFrameLength, jint frameLength) {
    JniInstance::getInstance()->setFrameLength(frameLength);
}

// Frame space

declare_binding(jint, getFrameSpace) {
    return JniInstance::getInstance()->getFrameSpace();
}

declare_binding2(setFrameSpace, jint frameSpace) {
    JniInstance::getInstance()->setFrameSpace(frameSpace);
}

// Window duration

declare_binding(jint, getDuration) {
    return JniInstance::getInstance()->getDuration();
}

declare_binding2(setDuration, jint duration) {
    JniInstance::getInstance()->setDuration(duration);
}

// Pitch algorithm

declare_binding(jobject, getPitchAlgs) {
    return JniInstance::getInstance()->getPitchAlgs();
}

declare_binding(jint, getPitchAlg) {
    return static_cast<jint>(JniInstance::getInstance()->getPitchAlg());
}

declare_binding2(setPitchAlg, jint algId) {
    JniInstance::getInstance()->setPitchAlg(static_cast<PitchAlg>(algId));
}

// Formant algorithm

declare_binding(jobject, getFormantAlgs) {
    return JniInstance::getInstance()->getFormantAlgs();
}

declare_binding(jint, getFormantAlg) {
    return static_cast<jint>(JniInstance::getInstance()->getFormantAlg());
}

declare_binding2(setFormantAlg, jint algId) {
    JniInstance::getInstance()->setFormantAlg(static_cast<FormantMethod>(algId));
}

// Min gain 

declare_binding(jint, getMinGain) {
    return JniInstance::getInstance()->getMinGain();
}

declare_binding2(setMinGain, jint minGain) {
    JniInstance::getInstance()->setMinGain(minGain);
}

// Max gain 

declare_binding(jint, getMaxGain) {
    return JniInstance::getInstance()->getMaxGain();
}

declare_binding2(setMaxGain, jint maxGain) {
    JniInstance::getInstance()->setMaxGain(maxGain);
}

// Pitch color

declare_binding(jint, getPitchColor) {
    return JniInstance::getInstance()->getPitchColor().rgb();
}

declare_binding2(setPitchColor, jint color) {
    JniInstance::getInstance()->setPitchColor(QColor::fromRgb(color));
}

// Formant color

declare_binding(jint, getFormantCount) {
    return JniInstance::getInstance()->getFormantCount();
}

declare_binding3(jint, getFormantColor, jint nb) {
    return JniInstance::getInstance()->getFormantColor(nb).rgb();
}

declare_binding2(setFormantColor, jint nb, jint color) {
    JniInstance::getInstance()->setFormantColor(nb, QColor::fromRgb(color));
}
