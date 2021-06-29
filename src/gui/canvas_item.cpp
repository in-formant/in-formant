#include "canvas.h"
#include <stdexcept>

using namespace Gui;

CanvasItem::CanvasItem()
    : mRenderContext(nullptr)
{
    setObjectName("IfCanvas");
    setTextureFollowsItemSize(true);
}

void CanvasItem::setRenderContext(Main::RenderContext *renderContext)
{
    mRenderContext = renderContext;
}

class CanvasInFboRenderer : public QQuickFramebufferObject::Renderer
{
public:
    CanvasInFboRenderer(Main::RenderContext *renderContext) {
        mRenderer.initialize(renderContext);
    }

    ~CanvasInFboRenderer() {
        mRenderer.cleanup();
    }

protected:
    void render() override {
        mRenderer.render();
        update();
    }
    
    void synchronize(QQuickFramebufferObject *item) override {
        mRenderer.synchronize(item);
    }

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override {
        QOpenGLFramebufferObjectFormat format;
        format.setSamples(4);
        return new QOpenGLFramebufferObject(size, format);
    }

private:
    CanvasRenderer mRenderer;
};

QQuickFramebufferObject::Renderer *CanvasItem::createRenderer() const
{
    return new CanvasInFboRenderer(mRenderContext);
}
