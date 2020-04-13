package fr.cloyunhee.speechanalysis;

import java.util.*;
import android.os.*;

public class JniBridge {

    public static native String getVersionString();
    public static native int getVersionCode();

    public static native void toggleAnalysis(boolean running);

    public static native int getFftSize();
    public static native void setFftSize(int fftSize);

    public static native int getLpOrder();
    public static native void setLpOrder(int lpOrder);

    public static native int getMaxFreq();
    public static native void setMaxFreq(int maxFreq);

    public static native int getFrameLength();
    public static native void setFrameLength(int frameLength);

    public static native int getFrameSpace();
    public static native void setFrameSpace(int frameSpace);

    public static native int getDuration();
    public static native void setDuration(int duration);

    public static native List<String> getPitchAlgs();
    public static native int getPitchAlg();
    public static native void setPitchAlg(int algId);

    public static native List<String> getFormantAlgs();
    public static native int getFormantAlg();
    public static native void setFormantAlg(int algId);

    public static native int getMinGain();
    public static native void setMinGain(int gain);

    public static native int getMaxGain();
    public static native void setMaxGain(int gain);

    public static native int getPitchColor();
    public static native void setPitchColor(int color);

    public static native int getFormantCount();
    public static native int getFormantColor(int nb);
    public static native void setFormantColor(int nb, int color);

    static {
        System.loadLibrary("speech_analysis_" + Build.SUPPORTED_ABIS[0]);
    }

}
