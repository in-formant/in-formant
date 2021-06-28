#ifndef QPAINTER_WRAPPER_H
#define QPAINTER_WRAPPER_H

#include "rpcxx.h"
#include "canvas_renderer.h"
#include "../timetrack.h"
#include "../context/datastore.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <QImage>

struct SpectrogramTextureData {
    double timeStart, timeEnd;
    double sampleRate;
    int nfft, segmentLen;
    rpm::vector<GLfloat> texture;
};

class QPainterWrapper {
public:
    QPainterWrapper(Gui::CanvasRenderer *p);
    
    QRect viewport() const;

    void setZoom(double scale);

    void setTimeRange(double start, double end);

    void setFrequencyScale(FrequencyScale scale);
    void setMinFrequency(double minFrequency);
    void setMaxFrequency(double maxFrequency);
    void setMaxGain(double maxGain);

    void drawTimeAxis();
    void drawFrequencyScale();

    void drawTimeSeries(const rpm::vector<double> &y, double xstart, double xend, double ymin, double ymax); 

    void drawFrequencyTrack(const TimeTrack<double>::const_iterator& begin,
                            const TimeTrack<double>::const_iterator& end,
                            float radius,
                            const QColor &color);

    void drawFrequencyTrack(const OptionalTimeTrack<double>::const_iterator& begin,
                            const OptionalTimeTrack<double>::const_iterator& end,
                            float radius,
                            const QColor &color);

    double mapTimeToX(double time);
    double mapFrequencyToY(double frequency);

    void drawSpectrogram(const rpm::vector<std::pair<double, Main::SpectrogramCoefs>>& slices);

    static double mapTimeToX(double time, int width, double startTime, double endTime);
    static double mapFrequencyToY(double frequency, int height, FrequencyScale scale, double minFrequency, double maxFrequency);
    static double mapYToFrequency(double y, int height, FrequencyScale scale, double minFrequency, double maxFrequency);

private:
    double transformFrequency(double frequency);
    double inverseFrequency(double value);
    
    Gui::CanvasRenderer *p;

    double mTimeStart;
    double mTimeEnd;

    FrequencyScale mFrequencyScale;
    double mMinFrequency;
    double mMaxFrequency;
    double mMaxGain;

    static double transformFrequency(double frequency, FrequencyScale scale);
    static double inverseFrequency(double value, FrequencyScale scale);

    static Eigen::SparseMatrix<double>& constructTransformY(int h, int vh, FrequencyScale freqScale, double freqMin, double freqMax, FrequencyScale sourceScale, double sourceMin, double sourceMax);

    static QVector<QRgb> cmap;
};

#endif // QPAINTER_WRAPPER_H
