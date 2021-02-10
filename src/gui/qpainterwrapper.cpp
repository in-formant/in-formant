#include "qpainterwrapper.h"
#include <Eigen/src/Core/Stride.h>
#include <QPainterPath>
#include <cmath>
#include <iostream>
#include <gaborator/gaussian.h>

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
    return log2(f);
}

inline double log2hz(double l) {
    return exp2(l);
}

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
        drawCurve(points);
        p->restore();
    }
}

QRgb QPainterWrapper::mapAmplitudeToColor(double amplitude)
{
    double gain = amplitude > 1e-10 ? 20 * log10(amplitude) : -1e6;
    double clampedGain = std::clamp(gain, mMinGain, mMaxGain);

    double clampedAmplitude = pow(10.0, clampedGain / 20);

    double a = sqrt(clampedAmplitude) * 7.5;

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

// ( vw, w )
using xtrans_key = std::tuple<int, int>;

// ( vh, h, scale, minf, maxf )
using ytrans_key = std::tuple<int, int, QPainterWrapper::FrequencyScale, double, double>;

static rpm::map<xtrans_key, Eigen::MatrixXd> xtrans_map;
static rpm::map<ytrans_key, Eigen::MatrixXd> ytrans_map;

void QPainterWrapper::constructTransformX(int w)
{
    const int vw = viewport().width();
    
    auto key = std::make_tuple(vw, w);
    auto it = xtrans_map.find(key);
    if (it != xtrans_map.end()) {
        mTransformX = it->second;
        return;
    }
  
    Eigen::MatrixXd xtrans(vw, w);
    xtrans.setZero();

    const double sd = log(2);

    for (int x = 0; x < w; ++x) {
        const double vxc = (double) (x * vw) / (double) w;
        for (int vx = 0; vx < vw; ++vx) {
            xtrans(vx, x) = gaborator::gaussian(sd, (double) (vx - vxc));
        }
    }

    xtrans_map.emplace(key, xtrans);
    mTransformX = xtrans;

    std::cout << " -- constructed x transform" << std::endl;
}

void QPainterWrapper::constructTransformY(int h)
{
    const int vh = viewport().height(); 

    auto key = std::make_tuple(vh, h, mFrequencyScale, mMinFrequency, mMaxFrequency);
    auto it = ytrans_map.find(key);
    if (it != ytrans_map.end()) {
        mTransformY = it->second;
        return;
    }

    const double lmin = hz2log(mMinFrequency);
    const double lmax = hz2log(mMaxFrequency); 

    const double vmin = transformFrequency(mMinFrequency);
    const double vmax = transformFrequency(mMaxFrequency);

    Eigen::MatrixXd ytrans(h, vh);
    ytrans.setZero();

    const double sd = (exp2(1.0 / 24) - 1) * 1000.0;

    for (int y = 0; y < h; ++y) {
        const double fc = log2hz(lmin + (1 - (double) y / (double) h) * (lmax - lmin));

        const double value = transformFrequency(fc);
        const double vyc = vh * (1 - (value - vmin) / (vmax - vmin));

        for (int vy = 0; vy < vh; ++vy) {
            ytrans(y, vy) = gaborator::norm_gaussian(sd, (vy - vyc) * 8);
        }
    }

    ytrans_map.emplace(key, ytrans);
    mTransformY = ytrans;

    std::cout << " -- constructed y transform" << std::endl;
}

QImage QPainterWrapper::drawSpectrogramChunk(const rpm::vector<double>& matrix, int width, int height)
{
    Eigen::Map<const Eigen::MatrixXd> logSpec(matrix.data(), width, height);
    
    constructTransformX(width);
    constructTransformY(height);
    
    Eigen::MatrixXd mapped = (/*mTransformX * */logSpec * mTransformY).transpose();

    QImage image(mapped.cols(), mapped.rows(), QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < mapped.rows(); ++y) {
        QRgb *scanLineBits = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < mapped.cols(); ++x) {
            scanLineBits[x] = mapAmplitudeToColor(mapped(y, x));
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

