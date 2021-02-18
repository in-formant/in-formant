#ifndef QPAINTER_WRAPPER_H
#define QPAINTER_WRAPPER_H

#include "rpcxx.h"
#include "../timetrack.h"
#include "qpainterwrapperbase.h"
#include <Eigen/Dense>

class QPainterWrapper : public QPainterWrapperBase {
public:
    enum class FrequencyScale : unsigned int {
        Linear      = 0,
        Logarithmic = 1,
        Mel         = 2,
    };

    QPainterWrapper(QPainter *p);
   
    void setTimeRange(double start, double end);

    void setFrequencyScale(FrequencyScale scale);
    void setMinFrequency(double minFrequency);
    void setMaxFrequency(double maxFrequency);
    void setMinGain(double minGain);
    void setMaxGain(double maxGain);

    void setMajorTickFont(const QFont &font);
    void setMajorTickPen(const QPen &pen);

    void setMinorTickFont(const QFont &font);
    void setMinorTickPen(const QPen &pen);

    void setTimeSeriesPen(const QPen &pen);

    void drawFrequencyScale();

    void drawTimeSeries(const rpm::vector<double> &y, double xstart, double xend, double ymin, double ymax); 

    void drawFrequencyTrack(const TimeTrack<double>::const_iterator& begin,
                            const TimeTrack<double>::const_iterator& end,
                            bool curve = false);

    void drawFrequencyTrack(const OptionalTimeTrack<double>::const_iterator& begin,
                            const OptionalTimeTrack<double>::const_iterator& end,
                            bool curve = true);

    void drawCurve(const rpm::vector<QPointF> &points, double tension = 0.5);

    double mapTimeToX(double time);
    double mapFrequencyToY(double frequency);

    static double mapTimeToX(double time, int width, double startTime, double endTime);
    static double mapFrequencyToY(double frequency, int height, FrequencyScale scale, double minFrequency, double maxFrequency);

    static QImage drawSpectrogram(const rpm::vector<double> &amplitudes,
            int width, int height, int viewportWidth, int viewportHeight,
            FrequencyScale scale, double minFrequency, double maxFrequency,
            double minGain, double maxGain);

private:
    double transformFrequency(double frequency);
    double inverseFrequency(double value);

    QRgb mapAmplitudeToColor(double amplitude);

    double mTimeStart;
    double mTimeEnd;

    FrequencyScale mFrequencyScale;
    double mMinFrequency;
    double mMaxFrequency;
    double mMinGain;
    double mMaxGain;

    QFont  mMajorTickFont;
    QPen   mMajorTickPen;

    QFont  mMinorTickFont;
    QPen   mMinorTickPen;

    QPen   mTimeSeriesPen;

    static double transformFrequency(double frequency, FrequencyScale scale);
    static double inverseFrequency(double value, FrequencyScale scale);

    static QRgb mapAmplitudeToColor(double amplitude, double minGain, double maxGain);

    static Eigen::MatrixXd constructTransformX(int w, int vw);
    static Eigen::MatrixXd constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax);

    static QVector<QRgb> cmap;
};

#endif // QPAINTER_WRAPPER_H
