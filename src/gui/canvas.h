#ifndef GUI_CANVAS_H
#define GUI_CANVAS_H

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

namespace Main {
    class RenderContext;
}

namespace Gui
{   
    class CanvasRenderer;

    class CanvasItem : public QQuickFramebufferObject
    {
        Q_OBJECT
    public:
        CanvasItem();

        void setRenderContext(Main::RenderContext *renderContext);
        
        Renderer *createRenderer() const override;

    private:
        Main::RenderContext *mRenderContext;
    };
}

#include "canvas_renderer.h"

#endif // GUI_CANVAS_H
