#include "qpainterwrapper.h"
#include <QPainterPath>
#include <cmath>
#include <gaborator/render.h>

constexpr double mapToUnit(double v, double min, double max) {
    return (v - min) / (max - min);
}

inline double hz2mel(double f) {
    return 2595.0 * log10(1.0 + f / 700.0);
}

inline double mel2hz(double m) {
    return 700.0 * (pow(10.0, m / 2595.0) - 1.0);
}

inline double hz2log(double f) {
    return log10(10.0 + f);
}

inline double log2hz(double l) {
    return pow(10.0, l) - 10.0;
}

QPainterWrapper::QPainterWrapper(QPainter *p)
    : QPainterWrapperBase(p),
      mTimeStart(0.0),
      mTimeEnd(5.0),
      mFrequencyScale(FrequencyScale::Mel),
      mMinFrequency(1),
      mMaxFrequency(8000),
      mMinGain(-120),
      mMaxGain(0)
{
}

void QPainterWrapper::setTimeRange(double start, double end)
{
    mTimeStart = start;
    mTimeEnd = end;
}

void QPainterWrapper::setFrequencyScale(FrequencyScale scale)
{
    mFrequencyScale = scale;
}

void QPainterWrapper::setMinFrequency(double minFrequency)
{
    mMinFrequency = minFrequency;
}

void QPainterWrapper::setMaxFrequency(double maxFrequency)
{
    mMaxFrequency = maxFrequency;
}

void QPainterWrapper::setMinGain(double minGain)
{
    mMinGain = minGain;
}

void QPainterWrapper::setMaxGain(double maxGain)
{
    mMaxGain = maxGain;
}

void QPainterWrapper::setMajorTickFont(const QFont &font)
{
    mMajorTickFont = font;
}

void QPainterWrapper::setMajorTickPen(const QPen &pen)
{
    mMajorTickPen = pen;
}

void QPainterWrapper::setMinorTickFont(const QFont &font)
{
    mMinorTickFont = font;
}

void QPainterWrapper::setMinorTickPen(const QPen &pen)
{
    mMinorTickPen = pen;
}

void QPainterWrapper::setTimeSeriesPen(const QPen &pen)
{
    mTimeSeriesPen = pen;
}

double QPainterWrapper::transformFrequency(double frequency)
{
    switch (mFrequencyScale) {
    case FrequencyScale::Linear:
        return frequency;
    case FrequencyScale::Logarithmic:
        return hz2log(frequency);
    case FrequencyScale::Mel:
        return hz2mel(frequency);
    default:
        return 0;
    }
}

double QPainterWrapper::inverseFrequency(double value)
{
    switch (mFrequencyScale) {
    case FrequencyScale::Linear:
        return value;
    case FrequencyScale::Logarithmic:
        return log2hz(value);
    case FrequencyScale::Mel:
        return mel2hz(value);
    default:
        return 0;
    }
}

double QPainterWrapper::mapTimeToX(double time)
{
    return p->viewport().width() * (time - mTimeStart) / (mTimeEnd - mTimeStart);
}

double QPainterWrapper::mapFrequencyToY(double frequency)
{
    const double min = transformFrequency(mMinFrequency);
    const double max = transformFrequency(mMaxFrequency);
    const double value = transformFrequency(frequency);

    return p->viewport().height() * (1 - (value - min) / (max - min));
}

void QPainterWrapper::drawFrequencyScale()
{
    
}

void QPainterWrapper::drawTimeSeries(const rpm::vector<double>& y, double xstart, double xend, double ymin, double ymax)
{
    const int len = y.size();
    const double xstep = (xend - xstart) / len;

    if (len > 0) {
        rpm::vector<QPointF> points(len);
        for (int i = 0; i < len; ++i) {
            points[i] = QPointF(
                xstart + i * xstep,
                p->viewport().height() * (y[i] - ymin) / (ymax - ymin)
            );
        }

        p->save();
        p->setPen(mTimeSeriesPen);
        drawCurve(points);
        p->restore();
    }
}

QRgb QPainterWrapper::mapAmplitudeToColor(double amplitude)
{
    double gain = amplitude > 1e-10 ? 20 * log10(amplitude) : -1e6;
    double clampedGain = std::clamp(gain, mMinGain, mMaxGain);

    double clampedAmplitude = pow(10.0, clampedGain / 20);

    double a = sqrt(clampedAmplitude) * 15;

    int leftIndex = std::floor(a * 255);

    if (leftIndex <= 0) {
        return cmap[0];
    }
    else if (leftIndex >= 255) {
        return cmap[255];
    }
    else {
        double frac = a * 255 - leftIndex;
        QColor c1 = QColor::fromRgb(cmap[leftIndex]);
        QColor c2 = QColor::fromRgb(cmap[leftIndex + 1]);
        int r = frac * c1.red() + (1 - frac) * c2.red();
        int g = frac * c1.green() + (1 - frac) * c2.green();
        int b = frac * c1.blue() + (1 - frac) * c2.blue();
        return qRgb(r, g, b);
    }
}

QImage QPainterWrapper::drawSpectrogramChunk(const rpm::vector<double>& matrix, int width, int height)
{
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < height; ++y) {
        QRgb *scanLineBits = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            scanLineBits[x] = mapAmplitudeToColor(matrix[y * width + x]);
        }
    }
    return image;
}

void QPainterWrapper::drawFrequencyTrack(
            const TimeTrack<double>::const_iterator& begin,
            const TimeTrack<double>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        double time = it->first;
        double pitch = it->second;

        double x = mapTimeToX(time);
        double y = mapFrequencyToY(pitch);

        p->drawPoint(x, y);
    }
}

static inline void cubicControlPoints(const QPointF &p1, const QPointF &p2, const QPointF &p3, double t, QPointF &ctrl1, QPointF &ctrl2)
{
    const double vx = p3.x() - p1.x();
    const double vy = p3.y() - p1.y();
    
    const double d01 = sqrt(QPointF::dotProduct(p1, p2));
    const double d12 = sqrt(QPointF::dotProduct(p2, p3));
    const double d012 = d01 + d12;

    ctrl1 = QPointF(
        p2.x() - (vx * t * d01) / d012,
        p2.y() - (vy * t * d01) / d012
    );
    ctrl2 = QPointF(
        p2.x() + (vx * t * d12) / d012,
        p2.y() + (vy * t * d12) / d012
    );
}

void QPainterWrapper::drawCurve(const rpm::vector<QPointF>& points, double tension)
{
    const int len = points.size();

    if (len == 1) {
        p->drawPoint(points[0]);
    }
    else if (len == 2) {
        p->drawLine(points[0], points[1]);
    }
    else {
        rpm::vector<QPointF> ctrlPoints(2 * (len - 2));
        for (int i = 0; i < len - 2; ++i) {
            cubicControlPoints(
                    points[i], points[i + 1], points[i + 2], tension,
                    ctrlPoints[2 * i], ctrlPoints[2 * i + 1]);
        }
        QPainterPath path;
        path.moveTo(points[0]);
        path.quadTo(ctrlPoints[0], points[1]);
        for (int i = 2; i < len - 1; ++i) {
            path.cubicTo(ctrlPoints[2 * (i - 1) - 1], ctrlPoints[2 * (i - 1)], points[i]);
        }
        path.quadTo(ctrlPoints[2 * (len - 2) - 1], points[len - 1]);
        p->drawPath(path);
    }
}

QVector<QRgb> QPainterWrapper::cmap = {
    0xff000003,
    0xff000004,
    0xff000006,
    0xff010007,
    0xff010109,
    0xff01010b,
    0xff02020d,
    0xff02020f,
    0xff030311,
    0xff040313,
    0xff040415,
    0xff050417,
    0xff060519,
    0xff07051b,
    0xff08061d,
    0xff09071f,
    0xff0a0722,
    0xff0b0824,
    0xff0c0926,
    0xff0d0a28,
    0xff0e0a2a,
    0xff0f0b2c,
    0xff100c2f,
    0xff110c31,
    0xff120d33,
    0xff140d35,
    0xff150e38,
    0xff160e3a,
    0xff170f3c,
    0xff180f3f,
    0xff1a1041,
    0xff1b1044,
    0xff1c1046,
    0xff1e1049,
    0xff1f114b,
    0xff20114d,
    0xff221150,
    0xff231152,
    0xff251155,
    0xff261157,
    0xff281159,
    0xff2a115c,
    0xff2b115e,
    0xff2d1060,
    0xff2f1062,
    0xff301065,
    0xff321067,
    0xff341068,
    0xff350f6a,
    0xff370f6c,
    0xff390f6e,
    0xff3b0f6f,
    0xff3c0f71,
    0xff3e0f72,
    0xff400f73,
    0xff420f74,
    0xff430f75,
    0xff450f76,
    0xff470f77,
    0xff481078,
    0xff4a1079,
    0xff4b1079,
    0xff4d117a,
    0xff4f117b,
    0xff50127b,
    0xff52127c,
    0xff53137c,
    0xff55137d,
    0xff57147d,
    0xff58157e,
    0xff5a157e,
    0xff5b167e,
    0xff5d177e,
    0xff5e177f,
    0xff60187f,
    0xff61187f,
    0xff63197f,
    0xff651a80,
    0xff661a80,
    0xff681b80,
    0xff691c80,
    0xff6b1c80,
    0xff6c1d80,
    0xff6e1e81,
    0xff6f1e81,
    0xff711f81,
    0xff731f81,
    0xff742081,
    0xff762181,
    0xff772181,
    0xff792281,
    0xff7a2281,
    0xff7c2381,
    0xff7e2481,
    0xff7f2481,
    0xff812581,
    0xff822581,
    0xff842681,
    0xff852681,
    0xff872781,
    0xff892881,
    0xff8a2881,
    0xff8c2980,
    0xff8d2980,
    0xff8f2a80,
    0xff912a80,
    0xff922b80,
    0xff942b80,
    0xff952c80,
    0xff972c7f,
    0xff992d7f,
    0xff9a2d7f,
    0xff9c2e7f,
    0xff9e2e7e,
    0xff9f2f7e,
    0xffa12f7e,
    0xffa3307e,
    0xffa4307d,
    0xffa6317d,
    0xffa7317d,
    0xffa9327c,
    0xffab337c,
    0xffac337b,
    0xffae347b,
    0xffb0347b,
    0xffb1357a,
    0xffb3357a,
    0xffb53679,
    0xffb63679,
    0xffb83778,
    0xffb93778,
    0xffbb3877,
    0xffbd3977,
    0xffbe3976,
    0xffc03a75,
    0xffc23a75,
    0xffc33b74,
    0xffc53c74,
    0xffc63c73,
    0xffc83d72,
    0xffca3e72,
    0xffcb3e71,
    0xffcd3f70,
    0xffce4070,
    0xffd0416f,
    0xffd1426e,
    0xffd3426d,
    0xffd4436d,
    0xffd6446c,
    0xffd7456b,
    0xffd9466a,
    0xffda4769,
    0xffdc4869,
    0xffdd4968,
    0xffde4a67,
    0xffe04b66,
    0xffe14c66,
    0xffe24d65,
    0xffe44e64,
    0xffe55063,
    0xffe65162,
    0xffe75262,
    0xffe85461,
    0xffea5560,
    0xffeb5660,
    0xffec585f,
    0xffed595f,
    0xffee5b5e,
    0xffee5d5d,
    0xffef5e5d,
    0xfff0605d,
    0xfff1615c,
    0xfff2635c,
    0xfff3655c,
    0xfff3675b,
    0xfff4685b,
    0xfff56a5b,
    0xfff56c5b,
    0xfff66e5b,
    0xfff6705b,
    0xfff7715b,
    0xfff7735c,
    0xfff8755c,
    0xfff8775c,
    0xfff9795c,
    0xfff97b5d,
    0xfff97d5d,
    0xfffa7f5e,
    0xfffa805e,
    0xfffa825f,
    0xfffb8460,
    0xfffb8660,
    0xfffb8861,
    0xfffb8a62,
    0xfffc8c63,
    0xfffc8e63,
    0xfffc9064,
    0xfffc9265,
    0xfffc9366,
    0xfffd9567,
    0xfffd9768,
    0xfffd9969,
    0xfffd9b6a,
    0xfffd9d6b,
    0xfffd9f6c,
    0xfffda16e,
    0xfffda26f,
    0xfffda470,
    0xfffea671,
    0xfffea873,
    0xfffeaa74,
    0xfffeac75,
    0xfffeae76,
    0xfffeaf78,
    0xfffeb179,
    0xfffeb37b,
    0xfffeb57c,
    0xfffeb77d,
    0xfffeb97f,
    0xfffebb80,
    0xfffebc82,
    0xfffebe83,
    0xfffec085,
    0xfffec286,
    0xfffec488,
    0xfffec689,
    0xfffec78b,
    0xfffec98d,
    0xfffecb8e,
    0xfffdcd90,
    0xfffdcf92,
    0xfffdd193,
    0xfffdd295,
    0xfffdd497,
    0xfffdd698,
    0xfffdd89a,
    0xfffdda9c,
    0xfffddc9d,
    0xfffddd9f,
    0xfffddfa1,
    0xfffde1a3,
    0xfffce3a5,
    0xfffce5a6,
    0xfffce6a8,
    0xfffce8aa,
    0xfffceaac,
    0xfffcecae,
    0xfffceeb0,
    0xfffcf0b1,
    0xfffcf1b3,
    0xfffcf3b5,
    0xfffcf5b7,
    0xfffbf7b9,
    0xfffbf9bb,
    0xfffbfabd,
    0xfffbfcbf,
};
