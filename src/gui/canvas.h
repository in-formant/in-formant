#ifndef GUI_CANVAS_H
#define GUI_CANVAS_H

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

namespace Gui
{

    class CanvasRenderer : public QQuickFramebufferObject::Renderer
    {
    protected:
        QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
        void render() override;
        void synchronize(QQuickFramebufferObject *item) override;
    };

    class Canvas : public QQuickFramebufferObject
    {
    public:
        Canvas(QQuickItem *parent = nullptr);
        QQuickFramebufferObject::Renderer *createRenderer() const override;
    };

}

#endif // GUI_CANVAS_H
