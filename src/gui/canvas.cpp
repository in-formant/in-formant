#include "../context/rendercontext.h"
#include "canvas.h"
#include "qpainterwrapper.h"
#include <QQuickWindow>
#include <QTimer>
#include <QScreen>
#include <chrono>

using namespace Gui;

CanvasItem::CanvasItem(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      mRenderContext(nullptr)
{
    setObjectName("IfCanvas");
    setFlag(ItemHasContents);
}

void CanvasItem::setRenderContext(Main::RenderContext *renderContext)
{
    mRenderContext = renderContext;
}

void CanvasItem::paint(QPainter *painterBase)
{
    QPainterWrapper painter(painterBase);
    mRenderContext->render(&painter);
}

