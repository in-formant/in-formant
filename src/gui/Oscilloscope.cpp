#include "Oscilloscope.h"
#include "../log/simpleQtLogger.h"

using namespace Eigen;

Oscilloscope::Oscilloscope()
{
    setObjectName("Oscilloscope");
    setFocusPolicy(Qt::StrongFocus);

    speechColor = Qt::black;
    speechThick = 1;

    sourceColor = 0xffa500;
    sourceThick = 1;

    loadSettings();
}

Oscilloscope::~Oscilloscope()
{
    saveSettings();
}

void Oscilloscope::paintEvent(QPaintEvent *event)
{
    targetWidth = width();
    targetHeight = height();
 
    painter.begin(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.fillRect(0, 0, targetWidth, targetHeight, Qt::white);
    
    renderUI();

    imageLock.lock();
    painter.drawPicture(0, 0, imSpeech); 
    painter.drawPicture(0, 0, imSource);
    imageLock.unlock();

    painter.end();
}

void Oscilloscope::renderUI()
{

}

void Oscilloscope::renderWaves(const ArrayXd& s, const ArrayXd& g)
{
    QPainterPath pathSpeech;
    QPainterPath pathSource;
    
    constexpr int sampleRate = 16000;
    const int length = s.size();

    int yZero = targetHeight / 2;
    int yOne = 0;

    for (int i = 0; i < length; ++i) {
        int x = (i * targetWidth) / length;

        int ySpeech = 0.5 * targetHeight * (1.0 - s(i));
        int ySource = 0.5 * targetHeight * (1.0 - g(i));

        if (i == 0) {
            pathSpeech.moveTo(x, ySpeech);
            pathSource.moveTo(x, ySource);
        }
        else {
            pathSpeech.lineTo(x, ySpeech);
            pathSource.lineTo(x, ySource);
        }
    }

    QPainter p;
    
    p.begin(&imSpeech);
    p.setPen(QPen(speechColor, speechThick));
    p.drawPath(pathSpeech);
    p.end();

    p.begin(&imSource);
    p.setPen(QPen(sourceColor, sourceThick));
    p.drawPath(pathSource);
    p.end();
}

void Oscilloscope::loadSettings(QSettings& settings)
{
    L_INFO("Loading oscilloscope settings...");

    if (settings.status() != QSettings::NoError) {
        L_INFO("Retrying loading settings");
        QTimer::singleShot(10, [&]() { loadSettings(settings); });
    }
    else {
        settings.beginGroup("oscilloscope");

        settings.endGroup();
    }
}

void Oscilloscope::saveSettings(QSettings& settings)
{
    L_INFO("Saving oscilloscope settings...");

    if (settings.status() != QSettings::NoError) {
        QTimer::singleShot(10, [&]() { loadSettings(settings); });
    }
    else {
        settings.beginGroup("oscilloscope");

        settings.endGroup();
    }
}
