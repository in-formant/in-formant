#ifndef DATAVIS_WRAPPER_H
#define DATAVIS_WRAPPER_H

#include <QObject>
#include "datastore.h"

#include <QtCharts>

namespace Main {

    class DataVisWrapper : public QObject {
        Q_OBJECT
        Q_PROPERTY(QVector<QPointF> sound   MEMBER mSound   NOTIFY soundChanged)
        Q_PROPERTY(QVector<QPointF> gif     MEMBER mGif     NOTIFY gifChanged)

    signals:
        void soundChanged();
        void gifChanged();

    public:
        DataVisWrapper();

        void setSound(const rpm::vector<double>& signal, double fs);
        void setGif(const rpm::vector<double>& signal, double fs);

        Q_INVOKABLE void updateSoundSeries(QXYSeries* series, QValueAxis* xAxis, QValueAxis* yAxis);
        Q_INVOKABLE void updateGifSeries(QXYSeries* series, QValueAxis* xAxis, QValueAxis* yAxis);
    
    private:
        QVector<QPointF> mSound;
        QMutex mSoundMutex;

        QVector<QPointF> mGif;
        double mGifStart, mGifEnd;
        QMutex mGifMutex;
    };

}

#endif
