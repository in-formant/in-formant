#include "rendercontext.h"
#include "config.h"
#include "../context/timings.h"
#include <iostream>

using namespace Main;

RenderContext::RenderContext(Config *config, DataStore *dataStore)
    : mConfig(config),
      mDataStore(dataStore),
      mSelectedView(nullptr)
{
}

void RenderContext::render(QPainterWrapper *painter)
{
    timer_guard timer(timings::render);
    
    painter->setRenderHints(
            QPainter::Antialiasing
            | QPainter::TextAntialiasing
            | QPainter::SmoothPixmapTransform);

    painter->setMinFrequency(mConfig->getViewMinFrequency());
    painter->setMaxFrequency(mConfig->getViewMaxFrequency());
    painter->setMinGain(mConfig->getViewMinGain());
    painter->setMaxGain(mConfig->getViewMaxGain());

    if (mSelectedView != nullptr) {
        mSelectedView->render(painter, mConfig, mDataStore);
    }

    painter->setPen(Qt::white);
    painter->setFont(QFont(":/Montserrat.otf", 20));
    painter->drawText(10, 80, QString("Render: %1 ms").arg(timings::render));
    painter->drawText(10, 110, QString("Update: %1 ms").arg(timings::update));
}

void RenderContext::setView(RenderView *view)
{
    mSelectedView = view;
}
