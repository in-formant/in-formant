#include "base.h"

using namespace Module::Renderer;

AbstractBase::AbstractBase(Type type)
    : mType(type),
      mDrawableWidth(1280),
      mDrawableHeight(720),
      mDrawableSizeChanged(false)
{
}

AbstractBase::~AbstractBase()
{
}

void AbstractBase::setDrawableSize(int width, int height)
{
    mDrawableWidth = width;
    mDrawableHeight = height;
    mDrawableSizeChanged = true;
}

void AbstractBase::getDrawableSize(int *pWidth, int *pHeight) const
{
    *pWidth = mDrawableWidth;
    *pHeight = mDrawableHeight;
}

bool AbstractBase::hasDrawableSizeChanged() const
{
    return mDrawableSizeChanged;
}

void AbstractBase::resetDrawableSizeChanged()
{
    mDrawableSizeChanged = false;
}
