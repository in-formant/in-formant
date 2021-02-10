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

    QImage drawSpectrogramChunk(const rpm::vector<double> &amplitudes, int width, int height);

    void drawFrequencyTrack(const TimeTrack<double>::const_iterator& begin,
                            const TimeTrack<double>::const_iterator& end);

    void drawCurve(const rpm::vector<QPointF> &points, double tension = 0.5);

private:
    double transformFrequency(double frequency);
    double inverseFrequency(double value);

    double mapTimeToX(double time);
    double mapFrequencyToY(double frequency);

    QRgb mapAmplitudeToColor(double amplitude);

    void constructTransformX(int w);
    void constructTransformY(int h);

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

    Eigen::MatrixXd mTransformX;
    Eigen::MatrixXd mTransformY;

    static QVector<QRgb> cmap;
};

#endif // QPAINTER_WRAPPER_H
