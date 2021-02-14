#include "qpainterwrapper.h"
#include <QPainterPath>

QPainterWrapper::QPainterWrapper(QPainter *p)
    : QPainterWrapperBase(p),
      mTimeStart(0.0),
      mTimeEnd(5.0),
      mFrequencyScale(FrequencyScale::Mel),
      mMinFrequency(60),
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
    return transformFrequency(frequency, mFrequencyScale);
}

double QPainterWrapper::inverseFrequency(double value)
{
    return inverseFrequency(value, mFrequencyScale);
}

double QPainterWrapper::mapTimeToX(double time)
{
    return mapTimeToX(time, p->viewport().width(), mTimeStart, mTimeEnd);
}

double QPainterWrapper::mapFrequencyToY(double frequency)
{
    return mapFrequencyToY(frequency, p->viewport().height(), mFrequencyScale, mMinFrequency, mMaxFrequency);
}

void QPainterWrapper::drawFrequencyScale()
{
    rpm::vector<double> majorTicks;
    rpm::vector<double> minorTicks;

    

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
        drawCurve(points, 0.8);
        p->restore();
    }
}

QRgb QPainterWrapper::mapAmplitudeToColor(double amplitude)
{
    return mapAmplitudeToColor(amplitude, mMinGain, mMaxGain);
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

