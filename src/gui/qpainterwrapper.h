#ifndef QPAINTER_WRAPPER_H
#define QPAINTER_WRAPPER_H

#include "rpcxx.h"
#include "../timetrack.h"
#include "../context/datastore.h"
#include "qpainterwrapperbase.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>

class QPainterWrapper : public QPainterWrapperBase {
public:
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

    void drawTimeAxis();
    void drawFrequencyScale();

    void drawTimeSeries(const rpm::vector<double> &y, double xstart, double xend, double ymin, double ymax); 

    void drawSpectrogram(const TimeTrack<Main::SpectrogramCoefs>::const_iterator& begin,
                         const TimeTrack<Main::SpectrogramCoefs>::const_iterator& end);

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
    static double mapYToFrequency(double y, int height, FrequencyScale scale, double minFrequency, double maxFrequency);

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

    static Eigen::SparseMatrix<double> constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax, FrequencyScale sourceScale, double sourceMin, double sourceMax);

    static QVector<QRgb> cmap;
};

#endif // QPAINTER_WRAPPER_H
