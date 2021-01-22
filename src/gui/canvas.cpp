#include "../context/rendercontext.h"
#include "canvas.h"
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QTimer>
#include <QScreen>
#include <chrono>

using namespace Gui;

Canvas::Canvas(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      mRenderContext(nullptr)
{
    setObjectName("ifCanvas");
    setMirrorVertically(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

QQuickFramebufferObject::Renderer *Canvas::createRenderer() const
{
    return new CanvasRenderer(mRenderContext);
}

void Canvas::setRenderContext(Main::RenderContext *renderContext)
{
    mRenderContext = renderContext;
}

CanvasRenderer::CanvasRenderer(Main::RenderContext *renderContext)
    : mRenderContext(renderContext)
{
}

QOpenGLFramebufferObject *CanvasRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    format.setSamples(2);
    return new QOpenGLFramebufferObject(size, format);
}

void CanvasRenderer::render()
{
    mItem->window()->resetOpenGLState();

    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        framebufferObject()->bind();
        mRenderContext->setWidth(mItem->width());
        mRenderContext->setHeight(mItem->height());
        mRenderContext->setDevicePixelRatio(mItem->window()->devicePixelRatio());
        mRenderContext->setDPI(mItem->window()->screen()->physicalDotsPerInchX(),
                               mItem->window()->screen()->physicalDotsPerInchY());
        mRenderContext->initialize();
        printf("Render context initialized\n");
    }
   
    mRenderContext->render();
}

void CanvasRenderer::synchronize(QQuickFramebufferObject *item)
{
    mItem = item;
}

void CanvasRenderer::connectEvents()
{
    /*
    QObject::connect(mItem, &QQuickItem::widthChanged, [=]() { pTarget->qtSetWidth(mItem->width()); });
    QObject::connect(mItem, &QQuickItem::heightChanged, [=]() { pTarget->qtSetHeight(mItem->height()); });
    QObject::connect(mItem->window(), &QWindow::screenChanged, pTarget, &Qt5Quick::qtSetDpi);

    auto initTimer = new QTimer;
    initTimer->setSingleShot(true);
    QObject::connect(initTimer, &QTimer::timeout, [=]() {
        pTarget->qtSetWidth(mItem->width());
        pTarget->qtSetHeight(mItem->height());
        pTarget->qtSetDpi(mItem->window()->screen());
    });
    initTimer->moveToThread(qApp->thread());
    QMetaObject::invokeMethod(initTimer, "start", Q_ARG(int, 0));

    pTarget->moveToThread(qApp->thread());

    mItem->installEventFilter(pTarget);
    mItem->window()->installEventFilter(pTarget);

    QObject::connect(mItem->window()->findChild<QObject *>("pitchTrack"),
            SIGNAL(togglePitchTrack(bool)), pManager.get(), SLOT(setDisplayPitchTrack(bool)));
    
    QObject::connect(mItem->window()->findChild<QObject *>("formantTracks"),
            SIGNAL(toggleFormantTracks(bool)), pManager.get(), SLOT(setDisplayFormantTracks(bool)));
    
    QObject::connect(pTarget, &Qt5Quick::requestClose, qApp, &QCoreApplication::quit);
    QObject::connect(pTarget, &Qt5Quick::togglePause, pManager.get(), &ContextManager::togglePaused);
    */

    /*auto updateTimer = new QTimer;
    updateTimer->setInterval(16ms);
    updateTimer->setTimerType(Qt::PreciseTimer);
    QObject::connect(updateTimer, &QTimer::timeout, mItem, &QQuickItem::update);
    updateTimer->moveToThread(qApp->thread());
    updateTimer->setParent(mItem);
    QMetaObject::invokeMethod(updateTimer, "start", Q_ARG(int, 0));*/

    /*QObject::connect(qApp, &QCoreApplication::aboutToQuit, [=]() {
                updateTimer->stop();
                pManagerMutex->lock();
                pManager->terminate();
                pManagerMutex->unlock();
            });*/
}

