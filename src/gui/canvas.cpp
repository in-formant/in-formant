#include "gui.h"
#include <QQuickWindow>
#include <QOpenGLContext>
#include <QTimer>
#include <QScreen>
#include <chrono>

using namespace std::chrono_literals;
using namespace Gui;

using Module::Target::Qt5Quick;
using Main::ContextManager;

Canvas::Canvas(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
{
    setObjectName("IfCanvas");
    setMirrorVertically(true);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

void Canvas::terminate()
{
    pManagerMutex->lock();
    pManager->terminate();
    pManagerMutex->unlock();
}

QQuickFramebufferObject::Renderer *Canvas::createRenderer() const
{
    return new CanvasRenderer;
}

QOpenGLFramebufferObject *CanvasRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    //format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}

void CanvasRenderer::render()
{
    pManagerMutex->lock();

    mItem->window()->resetOpenGLState();

    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        framebufferObject()->bind();
        pManager->start();
        connectEvents();
    }

    pManager->mainBody();

    pManagerMutex->unlock();
}

void CanvasRenderer::synchronize(QQuickFramebufferObject *item)
{
    mItem = item;
}

void CanvasRenderer::connectEvents()
{
    QObject::connect(mItem, &QQuickItem::widthChanged, [=]() { pTarget->qtSetWidth(mItem->width()); });
    QObject::connect(mItem, &QQuickItem::heightChanged, [=]() { pTarget->qtSetHeight(mItem->height()); });
    QObject::connect(mItem->window(), &QWindow::screenChanged, pTarget, &Qt5Quick::qtSetDpi);

    //QObject::connect(mItem->window(), SIGNAL(closing(QQuickCloseEvent *)), mItem, SLOT(terminate()));

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

    QObject::connect(pTarget, &Qt5Quick::requestClose, pManager.get(), &ContextManager::requestClose);
    QObject::connect(pTarget, &Qt5Quick::requestClose, qApp, &QCoreApplication::quit);
    QObject::connect(pTarget, &Qt5Quick::togglePause, pManager.get(), &ContextManager::togglePaused);
    QObject::connect(pTarget, &Qt5Quick::useFrameCursor, pManager.get(), &ContextManager::setUseFrameCursor);
    QObject::connect(pTarget, &Qt5Quick::displayLpSpec, pManager.get(), &ContextManager::setDisplayLpSpec);
    QObject::connect(pTarget, &Qt5Quick::displayPitchTracks, pManager.get(), &ContextManager::setDisplayPitchTracks);
    QObject::connect(pTarget, &Qt5Quick::displayFormantTracks, pManager.get(), &ContextManager::setDisplayFormantTracks);

    auto updateTimer = new QTimer;
    updateTimer->setInterval(33ms);
    updateTimer->setTimerType(Qt::PreciseTimer);
    QObject::connect(updateTimer, &QTimer::timeout, mItem, &QQuickItem::update);
    updateTimer->moveToThread(qApp->thread());
    updateTimer->setParent(mItem);
    QMetaObject::invokeMethod(updateTimer, "start", Q_ARG(int, 0));
}

