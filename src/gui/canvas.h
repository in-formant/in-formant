#ifndef GUI_CANVAS_H
#define GUI_CANVAS_H

#include <QQuickPaintedItem>
#include <QSGRenderNode>

namespace Main {
    class RenderContext;
}

namespace Gui
{
    class CanvasItem : public QQuickPaintedItem
    {
    public:
        CanvasItem(QQuickItem *parent = nullptr);

        void setRenderContext(Main::RenderContext *renderContext);

        void paint(QPainter *painter) override;

    private:
        Main::RenderContext *mRenderContext;
    };
}

#endif // GUI_CANVAS_H
