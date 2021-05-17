#include "qpainterwrapper.h"
#include <Eigen/src/Core/Array.h>
#include <QPainterPath>
#include <iostream>

QPainterWrapper::QPainterWrapper(Gui::CanvasRenderer *p)
    : p(p),
      mTimeStart(0.0),
      mTimeEnd(5.0),
      mFrequencyScale(FrequencyScale::Mel),
      mMinFrequency(60),
      mMaxFrequency(8000),
      mMaxGain(0)
{
}

QRect QPainterWrapper::viewport() const
{
    return p->viewport();
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

static std::string numberToString(double val)
{
    std::stringstream ss;
    ss << val;
    return ss.str();
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
    
    for (const double val : majorTicks) {
        const double x = mapTimeToX(val);
        const auto valstr = numberToString(val);
        QRect rect = p->textBoundsSmall(valstr);
        rect.translate(x - rect.width() / 2, y1 - 10);
        bool covered = false;
        for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
            if (tx >= 0 && tx < bits.size()
                    && bits[tx]) {
                covered = true;
                break;
            }
        }
        p->drawLine(x, y1, x, y1 - 8, Qt::white, 3);
        if (!covered && val >= 0) {
            p->drawTextSmallOutlined(x - rect.width() / 2, y1 - 10, Qt::white, valstr, Qt::black);
            for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
                if (tx >= 0 && tx < bits.size())
                    bits[tx] = true;
            }
        }
    }

    for (const double val : minorTicks) {
        const double x = mapTimeToX(val);
        const auto valstr = numberToString(val);
        QRect rect = p->textBoundsSmaller(valstr);
        rect.translate(x - rect.width() / 2, y1 - 10);
        bool covered = false;
        for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
            if (tx >= 0 && tx < bits.size()
                    && bits[tx]) {
                covered = true;
                break;
            }
        }
        p->drawLine(x, y1, x, y1 - 4, Qt::white, 2);
        if (!covered) {
            p->drawTextSmallerOutlined(x - rect.width() / 2, y1 - 10, Qt::white, valstr, Qt::black);
            for (int tx = rect.x(); tx <= rect.x() + rect.width(); ++tx) {
                if (tx >= 0 && tx < bits.size())
                    bits[tx] = true;
            }
        }
    }
    
    for (const double val : minorMinorTicks) {
        const double x = mapTimeToX(val);
        p->drawLine(x, y1, x, y1 - 2, Qt::white, 1.5);
    }
}

void QPainterWrapper::drawFrequencyScale()
{
    rpm::vector<double> majorTicks;
    rpm::vector<double> minorTicks;
    rpm::vector<double> minorMinorTicks;

    if (mFrequencyScale == FrequencyScale::Linear) {
        int loFreq = std::floor(mMinFrequency / 1000) * 1000;
        int hiFreq = std::ceil(mMaxFrequency / 1000) * 1000;

        for (int freqInt = loFreq; freqInt <= hiFreq; freqInt += 1000) {
            majorTicks.push_back(freqInt);

            for (int division = 1; division <= 9; ++division) {
                const double freq = freqInt + division * 100.0;
                if (division == 5)
                    minorTicks.push_back(freq);
                else
                    minorMinorTicks.push_back(freq);
            }
        }
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
    
    for (const double val : majorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = numberToString(val);
        QRect rect = p->textBoundsNormal(valstr);
        rect.translate(x1 - 12 - rect.width(), y + rect.height() / 2);
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
        p->drawLine(x1 - 8, y, x1, y, Qt::white, 3);
        p->drawTextNormalOutlined(rect.x(), rect.y(), Qt::white, valstr, Qt::black);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    for (const double val : minorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = numberToString(val);
        QRect rect = p->textBoundsSmall(valstr);
        rect.translate(x1 - 12 - rect.width(), y + rect.height() / 2);
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
        p->drawLine(x1 - 6, y, x1, y, Qt::white, 2);
        p->drawTextSmallOutlined(rect.x(), rect.y(), Qt::white, valstr, Qt::black);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }

    for (const double val : minorMinorTicks) {
        const double y = mapFrequencyToY(val);
        const auto valstr = numberToString(val);
        QRect rect = p->textBoundsSmaller(valstr);
        rect.translate(x1 - 12 - rect.width(), y + rect.height() / 2);
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
        p->drawLine(x1 - 4, y, x1, y, Qt::white, 2);
        p->drawTextSmallerOutlined(rect.x(), rect.y(), Qt::white, valstr, Qt::black);
        for (int ty = rect.y(); ty <= rect.y() + rect.height(); ++ty) {
            if (ty >= 0 && ty < bits.size())
                bits[ty] = true;
        }
    }
}

void QPainterWrapper::drawFrequencyTrack(
            const TimeTrack<double>::const_iterator& begin,
            const TimeTrack<double>::const_iterator& end,
            float radius,
            const QColor &color)
{
    rpm::vector<QPointF> points;

    for (auto it = begin; it != end; ++it) {
        double time = it->first;
        double pitch = it->second;

        double x = mapTimeToX(time);
        double y = mapFrequencyToY(pitch);

        points.emplace_back(x, y);
    }

    p->drawScatterWithOutline(points, radius, color);
}

void QPainterWrapper::drawFrequencyTrack(
            const OptionalTimeTrack<double>::const_iterator& begin,
            const OptionalTimeTrack<double>::const_iterator& end,
            float radius,
            const QColor &color)
{
    rpm::vector<QPointF> points;

    for (auto it = begin; it != end; ++it) {
        if (it->second.has_value()) {
            double time = it->first;
            double pitch = *(it->second);

            double x = mapTimeToX(time);
            double y = mapFrequencyToY(pitch);

            points.emplace_back(x, y);
        }
    }

    p->drawScatterWithOutline(points, radius, color);
}

