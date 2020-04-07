#ifndef POWER_SPECTRUM_H
#define POWER_SPECTRUM_H

#include <QtWidgets>
#include <QTimer>
#include "../analysis/Analyser.h"

class PowerSpectrum : public QWidget {
    Q_OBJECT
public:
    PowerSpectrum(Analyser * analyser);
    ~PowerSpectrum();

protected:
    void paintEvent(QPaintEvent * event) override;

private:
    void renderSpectrum(int nframe, int nNew, double maximumFrequency, std::deque<SpecFrame>::const_iterator begin, std::deque<SpecFrame>::const_iterator end);

    double frequencyFromY(int y, double maximumFrequency);
    int yFromFrequency(double freq, double maximumFrequency);

    std::vector<SpecFrame> hold;
    int holdLength, holdIndex;

    QTimer timer;
   
    QPainter painter;
    int targetWidth, targetHeight;

    int minGain, maxGain;

    Analyser * analyser;
};

#endif // POWER_SPECTRUM_H
