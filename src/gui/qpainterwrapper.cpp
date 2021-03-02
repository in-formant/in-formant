#include "qpainterwrapper.h"
#include <QPainterPath>
#include <iostream>

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
    rpm::vector<double> minorMinorTicks;

    if (mFrequencyScale == FrequencyScale::Linear) {
    }
    else {
        double loLog = log10(mMinFrequency);
        double hiLog = log10(mMaxFrequency);
        int loDecade = (int) floor(loLog);

        double val;
        double startDecade = pow(10.0, (double) loDecade);

        // Major ticks are the decades.
        double decade = startDecade;
        double delta = hiLog - loLog, steps = fabs(delta);
        double step = delta >= 0 ? 10 : 0.1;
        double rMin = std::min(mMinFrequency, mMaxFrequency);
        double rMax = std::max(mMinFrequency, mMaxFrequency);
        for (int i = 0; i <= steps; ++i) { 
            val = decade;
            if (val >= rMin && val < rMax) {
                majorTicks.push_back(val);
            }
            decade *= step;
        }

        // Minor ticks are multiple of decades.
        decade = startDecade;
        float start, end, mstep;
        if (delta > 0) {
            start = 2; end = 9; mstep = 1;
        }
        else {
            start = 9; end = 2; mstep = -1;
        }
        ++steps;
        for (int i = 0; i <= steps; ++i) {
            for (int j = start; mstep > 0 ? j <= end : j >= end; j += mstep) {
                val = decade * j;
                if (val >= rMin && val < rMax) {
                    minorTicks.push_back(val);
                }
            }
            decade *= step;
        }

        // MinorMinor ticks are multiple of decades.
        decade = startDecade;
        if (delta > 0) {
            start = 10; end = 100; mstep = 1;
        }
        else {
            start = 100; end = 10; mstep = -1;
        }
        ++steps;
        for (int i = 0; i <= steps; ++i) {
            if (decade >= 10.0) {
                for (int f = start; mstep > 0 ? f <= end : f >= end; f += mstep) {
                    if ((int) (f / 10) != f / 10.0) {
                        val = decade * f / 10;
                        if (val >= rMin && val < rMax) {
                            minorMinorTicks.push_back(val);
                        }
                    }
                }
            }
            decade *= step;
        }
    }

    int x1 = viewport().width();
    std::vector<bool> bits(viewport().height(), false);

    QFont tickFont(p->font());
    
    p->setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
    tickFont.setPointSize(13);
    p->setFont(tickFont);
    for (const double val : majorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = QString("%1").arg(val, 'g');
        const int horizAdvance = p->fontMetrics().horizontalAdvance(valstr);
        const int descent = p->fontMetrics().descent();
        const int fontHeight = p->fontMetrics().height();
        QRect rect(x1 - 12 - horizAdvance, y + descent, horizAdvance, fontHeight);
        bool covered = false;
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size()
                    && bits[ty]) {
                covered = true;
                break;
            }
        }
        if (covered) {
            continue;
        }
        p->drawLine(x1 - 8, y, x1, y);
        p->drawText(rect.x(), rect.y(), valstr);

        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    p->setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap));
    tickFont.setPointSize(11);
    p->setFont(tickFont);
    for (const double val : minorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = QString("%1").arg(val, 'g');
        const int horizAdvance = p->fontMetrics().horizontalAdvance(valstr);
        const int descent = p->fontMetrics().descent();
        const int fontHeight = p->fontMetrics().height();
        QRect rect(x1 - 10 - horizAdvance, y + descent, horizAdvance, fontHeight);
        bool covered = false;
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size()
                    && bits[ty]) {
                covered = true;
                break;
            }
        }
        if (covered) {
            continue;
        }
        p->drawLine(x1 - 6, y, x1, y);
        p->drawText(rect.x(), rect.y(), valstr);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    tickFont.setPointSize(10);
    p->setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    for (const double val : minorMinorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = QString("%1").arg(val, 'g');
        const int horizAdvance = p->fontMetrics().horizontalAdvance(valstr);
        const int descent = p->fontMetrics().descent();
        const int fontHeight = p->fontMetrics().height();
        QRect rect(x1 - 8 - horizAdvance, y + descent, horizAdvance, fontHeight);
        bool covered = false;
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size()
                    && bits[ty]) {
                covered = true;
                break;
            }
        }
        if (covered) {
            continue;
        }
        p->drawLine(x1 - 4, y, x1, y);
        p->drawText(rect.x(), rect.y(), valstr);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }
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

void QPainterWrapper::drawSpectrogram(
            const TimeTrack<Main::SpectrogramCoefs>::const_iterator& begin,
            const TimeTrack<Main::SpectrogramCoefs>::const_iterator& end)
{
    int vh = viewport().height();
  
    int numBins = vh;
    int numSlices = std::distance(begin, end);
    
    if (numBins == 0 || numSlices == 0) {
        return;
    }

    QImage image(numSlices, numBins, QImage::Format_RGB32);

    int ix = 0;

    for (auto it = begin; it != end; ++it) {
        double time = it->first;
        const auto& coefs = it->second;

        auto& slice = coefs.magnitudes;

        auto ytrans = constructTransformY(slice.rows(), numBins, mFrequencyScale, mMinFrequency, mMaxFrequency, FrequencyScale::Linear, 0, coefs.sampleRate / 2);
          
        Eigen::VectorXd mapped = (ytrans * slice).reverse();

        for (int vy = 0; vy < mapped.size(); ++vy) {
            QRgb *scanLineBits = reinterpret_cast<QRgb *>(image.scanLine(vy));
            scanLineBits[ix] = mapAmplitudeToColor(mapped(vy));
        }

        ix++;
    }
   
    double xstart = mapTimeToX(begin->first);
    double xend = mapTimeToX(std::prev(end)->first);

    p->drawImage(xstart, 0, image.scaled(xend - xstart, vh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void QPainterWrapper::drawFrequencyTrack(
            const TimeTrack<double>::const_iterator& begin,
            const TimeTrack<double>::const_iterator& end,
            bool curve)
{
    rpm::vector<QPointF> points;

    for (auto it = begin; it != end; ++it) {
        double time = it->first;
        double pitch = it->second;

        double x = mapTimeToX(time);
        double y = mapFrequencyToY(pitch);

        points.emplace_back(x, y);
    }

    if (curve) {
        drawCurve(points);
    }
    else {
        p->drawPoints(points.data(), points.size());
    }
}

void QPainterWrapper::drawFrequencyTrack(
            const OptionalTimeTrack<double>::const_iterator& begin,
            const OptionalTimeTrack<double>::const_iterator& end,
            bool curve)
{
    rpm::vector<rpm::vector<QPointF>> segments;
    rpm::vector<QPointF> points;

    for (auto it = begin; it != end; ++it) {
        if (it->second.has_value()) {
            double time = it->first;
            double pitch = *(it->second);

            double x = mapTimeToX(time);
            double y = mapFrequencyToY(pitch);

            points.emplace_back(x, y);
        }
        else if (!points.empty()) {
            segments.push_back(std::move(points));
        }
    }

    if (!points.empty()) {
        segments.push_back(std::move(points));
    }

    if (curve) {
        for (const auto& segmentPoints : segments) {
            drawCurve(segmentPoints);
        }
    }
    else {
        for (const auto& segmentPoints : segments) {
            p->drawPoints(segmentPoints.data(), segmentPoints.size());
        }
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

