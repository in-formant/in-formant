#include "canvas.h"

using namespace Gui;

Canvas::Canvas(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
}

QQuickFramebufferObject::Renderer *Canvas::createRenderer() const
{
    return new CanvasRenderer;
}

QOpenGLFramebufferObject *CanvasRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void CanvasRenderer::render()
{
}

void CanvasRenderer::synchronize(QQuickFramebufferObject *item)
{
}
