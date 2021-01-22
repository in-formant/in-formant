#include "rendercontext.h"
#include "config.h"

using namespace Main;

RenderContext::RenderContext(Config *config, DataStore *dataStore)
    : mRenderer(std::make_unique<Module::Renderer::NanoVG>()),
      mFreetype(std::make_unique<FTInstance>()),
      mFontFile(mFreetype->font("Montserrat.otf")),
      mConfig(config),
      mDataStore(dataStore),
      mSelectedView(nullptr),
      mWidth(320),
      mHeight(240),
      mDevicePixelRatio(1.0)
{
    mRenderer->setProvider(&mRenderContextProvider);
}

void RenderContext::initialize()
{
    mRenderer->initialize();

    auto p = mRenderer->getParameters();
    p->setMinFrequency(mConfig->getViewMinFrequency());
    p->setMaxFrequency(mConfig->getViewMaxFrequency());
    p->setMinGain(mConfig->getViewMinGain());
    p->setMaxGain(mConfig->getViewMaxGain());
    p->setFrequencyScale(mConfig->getViewFrequencyScale());
}

void RenderContext::terminate()
{
    mRenderer->terminate();
}

void RenderContext::render()
{
    mRenderer->begin();
    if (mSelectedView != nullptr) {
        mSelectedView->render(mRenderer.get(), this, mConfig, mDataStore);
    }
    mRenderer->end();
}

void RenderContext::setView(RenderView *view)
{
    mSelectedView = view;
}

void RenderContext::setWidth(int width)
{
    mWidth = width;
    updateSize();
}

void RenderContext::setHeight(int height)
{
    mHeight = height;
    updateSize();
}

void RenderContext::setDevicePixelRatio(double ratio)
{
    mDevicePixelRatio = ratio;
    updateSize();
}

void RenderContext::setDPI(double horizontalDpi, double verticalDpi)
{
    mHorizontalDpi = horizontalDpi;
    mVerticalDpi = verticalDpi;
    updateSize();
}

Font& RenderContext::font(int pointSize)
{
    return mFontFile.with(pointSize, mRenderer->getContextNumber(), mHorizontalDpi, mVerticalDpi);
}

void RenderContext::updateSize()
{
    mRenderer->setWindowSize(mWidth, mHeight);
    mRenderer->setDrawableSize(mWidth, mHeight);
}
