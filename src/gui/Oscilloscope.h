#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <Eigen/Dense>
#include <QtWidgets>
#include "rpmalloc.h"

class Oscilloscope : public QWidget {
    Q_OBJECT
public:
    Oscilloscope();
    ~Oscilloscope();

    void loadSettings() { QSettings s; loadSettings(s); }
    void saveSettings() { QSettings s; saveSettings(s); }

protected:
    void paintEvent(QPaintEvent *event) override;

public slots:
    void renderUI();
    void renderWaves(const Eigen::ArrayXd& x, const Eigen::ArrayXd& g);

private:
    void loadSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

    std::mutex imageLock;
    QPicture imSpeech;
    QPicture imSource;

    QColor speechColor;
    QColor sourceColor;
    int speechThick;
    int sourceThick;

    QPainter painter;
    int targetWidth, targetHeight;
};

#endif // OSCILLOSCOPE
