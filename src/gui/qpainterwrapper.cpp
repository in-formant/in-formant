#include "qpainterwrapper.h"
#include <Eigen/src/Core/Array.h>
#include <QPainterPath>
#include <iostream>
#include <qnamespace.h>

QPainterWrapper::QPainterWrapper(QPainter *p)
    : QPainterWrapperBase(p),
      mTimeStart(0.0),
      mTimeEnd(5.0),
      mFrequencyScale(FrequencyScale::Mel),
      mMinFrequency(60),
      mMaxFrequency(8000),
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

void QPainterWrapper::drawTimeAxis()
{
    rpm::vector<double> majorTicks;
    rpm::vector<double> minorTicks;
    rpm::vector<double> minorMinorTicks;

    int timeStart = std::floor(mTimeStart);
    int timeEnd = std::ceil(mTimeEnd);

    for (int timeInt = timeStart; timeInt <= timeEnd; ++timeInt) {
        majorTicks.push_back(timeInt);
        
        // No need to be so detailed for negative time stamps.
        if (timeInt < 0)
            continue;

        for (int division = 1; division <= 9; ++division) {
            const double time = timeInt + division / 10.0;
            if (division == 5)
                minorTicks.push_back(time);
            else
                minorMinorTicks.push_back(time);
        }
    }

    int y1 = viewport().height();
    std::vector<bool> bits(viewport().width(), false);

    QFont tickFont(p->font());
    
    p->setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
    tickFont.setPointSize(13);
    p->setFont(tickFont);
    for (const double val : majorTicks) {
        const double x = mapTimeToX(val);
        const auto valstr = QString::number(val, 'g');
        const int horizAdvance = p->fontMetrics().horizontalAdvance(valstr);
        const int descent = p->fontMetrics().descent();
        const int fontHeight = p->fontMetrics().height();
        QRect rect(x - horizAdvance / 2, y1 - 10 - descent, horizAdvance, fontHeight);
        bool covered = false;
        for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
            if (tx >= 0 && tx < bits.size()
                    && bits[tx]) {
                covered = true;
                break;
            }
        }
        p->setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
        p->drawLine(x, y1, x, y1 - 8);
        if (!covered) {
            QPainterPath textPath;
            textPath.addText(rect.x(), rect.y(), tickFont, valstr);
            p->strokePath(textPath, QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap));
            p->fillPath(textPath, Qt::white);
            for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
                if (tx >= 0 && tx < bits.size())
                    bits[tx] = true;
            }
        }
    }

    p->setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap));
    tickFont.setPointSize(11);
    p->setFont(tickFont);
    for (const double val : minorTicks) {
        const double x = mapTimeToX(val);
        const auto valstr = QString::number(val, 'g');
        const int horizAdvance = p->fontMetrics().horizontalAdvance(valstr);
        const int descent = p->fontMetrics().descent();
        const int fontHeight = p->fontMetrics().height();
        QRect rect(x - horizAdvance / 2, y1 - 10 - descent, horizAdvance, fontHeight);
        bool covered = false;
        for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
            if (tx >= 0 && tx < bits.size()
                    && bits[tx]) {
                covered = true;
                break;
            }
        }
        p->drawLine(x, y1, x, y1 - 4);
        if (!covered) {
            QPainterPath textPath;
            textPath.addText(rect.x(), rect.y(), tickFont, valstr);
            p->strokePath(textPath, QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap));
            p->fillPath(textPath, Qt::white);
            for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
                if (tx >= 0 && tx < bits.size())
                    bits[tx] = true;
            }
        }
    }
    
    p->setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
    for (const double val : minorMinorTicks) {
        const double x = mapTimeToX(val);
        p->drawLine(x, y1, x, y1 - 2);
    }
}

QImage QPainterWrapper::drawFrequencyScale()
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

    QImage image(viewport().width(), viewport().height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter imp(&image);
    imp.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    int x1 = viewport().width();
    std::vector<bool> bits(viewport().height(), false);

    QFont tickFont(p->font());
    
    tickFont.setPointSize(13);
    imp.setFont(tickFont);
    for (const double val : majorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = QString::number(val, 'g');
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
        imp.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
        imp.drawLine(x1 - 8, y, x1, y);
        QPainterPath textPath;
        textPath.addText(rect.x(), rect.y(), tickFont, valstr);
        imp.strokePath(textPath, QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
        imp.fillPath(textPath, Qt::white);

        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    tickFont.setPointSize(11);
    imp.setFont(tickFont);
    for (const double val : minorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = QString::number(val, 'g');
        const int horizAdvance = imp.fontMetrics().horizontalAdvance(valstr);
        const int descent = imp.fontMetrics().descent();
        const int fontHeight = imp.fontMetrics().height();
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
        imp.setPen(QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap));
        imp.drawLine(x1 - 6, y, x1, y);
        QPainterPath textPath;
        textPath.addText(rect.x(), rect.y(), tickFont, valstr);
        imp.strokePath(textPath, QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
        imp.fillPath(textPath, Qt::white);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    tickFont.setPointSize(10);
    for (const double val : minorMinorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = QString::number(val, 'g');
        const int horizAdvance = imp.fontMetrics().horizontalAdvance(valstr);
        const int descent = imp.fontMetrics().descent();
        const int fontHeight = imp.fontMetrics().height();
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
        imp.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
        imp.drawLine(x1 - 4, y, x1, y);
        QPainterPath textPath;
        textPath.addText(rect.x(), rect.y(), tickFont, valstr);
        imp.strokePath(textPath, QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
        imp.fillPath(textPath, Qt::white);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    imp.end();
    return image;
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

QImage QPainterWrapper::drawFrequencyTrack(
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

    QImage image(viewport().width(), viewport().height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter imp(&image);
    imp.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QPen pen = p->pen();
    QPen outlinePen = pen;
    outlinePen.setColor(Qt::black);
    pen.setWidthF(pen.widthF() - 2.5);

    if (curve) {
        imp.setPen(outlinePen);
        drawCurve(points);
        imp.setPen(pen);
        drawCurve(points);
    }
    else {
        imp.setPen(outlinePen);
        imp.drawPoints(points.data(), points.size());
        imp.setPen(pen);
        imp.drawPoints(points.data(), points.size());
    }

    imp.end();
    return image;
}

QImage QPainterWrapper::drawFrequencyTrack(
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

    QImage image(viewport().width(), viewport().height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter imp(&image);
    imp.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QPen pen = p->pen();
    QPen outlinePen = pen;
    outlinePen.setColor(Qt::black);
    pen.setWidthF(pen.widthF() - 2.5);

    if (curve) {
        imp.setPen(outlinePen);
        for (const auto& segmentPoints : segments) {
            drawCurve(segmentPoints);
        }
        
        imp.setPen(pen);
        for (const auto& segmentPoints : segments) {
            drawCurve(segmentPoints);
        }
    }
    else {
        imp.setPen(outlinePen);
        for (const auto& segmentPoints : segments) {
            imp.drawPoints(segmentPoints.data(), segmentPoints.size());
        }

        imp.setPen(pen);
        for (const auto& segmentPoints : segments) {
            imp.drawPoints(segmentPoints.data(), segmentPoints.size());
        }
    }

    imp.end();
    return image;
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

