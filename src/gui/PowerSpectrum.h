#ifndef POWER_SPECTRUM_H
#define POWER_SPECTRUM_H

#include <QtWidgets>
#include <QTimer>
#include <mutex>
#include "rpmalloc.h"
#include "../analysis/Analyser.h"
#include "../gui/AnalyserCanvas.h"

class PowerSpectrum : public QWidget {
    Q_OBJECT
public:
    PowerSpectrum(Analyser * analyser, AnalyserCanvas * canvas);

protected:
    void paintEvent(QPaintEvent * event) override;

public slots:
    void renderSpectrum(int nframe, int nNew, double maximumFrequency, rpm::deque<SpecFrame>::const_iterator begin, rpm::deque<SpecFrame>::const_iterator end);
    void renderLpc(double maxFreq, SpecFrame lpcSpectrum);

private:
    double frequencyFromY(int y, double maximumFrequency);
    int yFromFrequency(double freq, double maximumFrequency);

    rpm::vector<SpecFrame> hold;
    int holdLength, holdIndex;

    std::mutex imageLock;
    QImage spectrum;
    QImage lpcIm;
   
    QPainter painter;
    int targetWidth, targetHeight;

    Analyser * analyser;
    AnalyserCanvas * canvas;
};

#endif // POWER_SPECTRUM_H
