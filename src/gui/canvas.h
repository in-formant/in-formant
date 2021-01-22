#ifndef GUI_CANVAS_H
#define GUI_CANVAS_H

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

namespace Main {
    class RenderContext;
}

namespace Gui
{

    class CanvasRenderer : public QQuickFramebufferObject::Renderer
    {
    public:
        CanvasRenderer(Main::RenderContext *renderContext);

    protected:
        QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
        void render() override;
        void synchronize(QQuickFramebufferObject *item) override;
        void setRenderContext(Main::RenderContext *renderContext);

    private:
        void connectEvents();
        
        Main::RenderContext *mRenderContext;
        
        QQuickFramebufferObject *mItem;
    };

    class Canvas : public QQuickFramebufferObject
    {
    public:
        Canvas(QQuickItem *parent = nullptr);
        QQuickFramebufferObject::Renderer *createRenderer() const override;
        void setRenderContext(Main::RenderContext *renderContext);

    private:
        Main::RenderContext *mRenderContext;
    };

}

#endif // GUI_CANVAS_H
