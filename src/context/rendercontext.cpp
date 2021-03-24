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

    painter->setMinFrequency(mConfig->getViewMinFrequency());
    painter->setMaxFrequency(mConfig->getViewMaxFrequency());
    painter->setFrequencyScale(mConfig->getViewFrequencyScale());
    painter->setMaxGain(mConfig->getViewMaxGain());

    if (mSelectedView != nullptr) {
        mSelectedView->render(painter, mConfig, mDataStore);
    }
}

void RenderContext::setView(RenderView *view)
{
    mSelectedView = view;
}
