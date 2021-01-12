#include "qt5quick.h"

#include <cmath>
#include <cassert>
#include <QQuickWindow>
#include <QScreen>
#include <QMutexLocker>

using namespace Module::Target;

Qt5Quick *pTarget;

Qt5Quick::Qt5Quick(Type rendererType)
    : AbstractBase { Type::NanoVG }
{
    assert(rendererType == Type::NanoVG);

    pTarget = this;

#ifdef RENDERER_USE_NVG
    setNvgProvider(new Q5Q_NanoVG);
#endif
}

Qt5Quick::~Qt5Quick()
{
}

void Qt5Quick::initialize()
{
    mWidth = mHeight = 100;
    mDpiHoriz = mDpiVert = 96;
    mDpiDiag = 135.764501988;
    mMouseDown = false;
    mMouseX = mMouseY = 100;
    mSizeChanged = false;
}

void Qt5Quick::terminate()
{
    // Nothing to do here.
}

void Qt5Quick::setTitle(const std::string& title)
{
    // Ignore title changes.
}

void Qt5Quick::setSize(int width, int height)
{
    // Ignore manual resize requests.
}

void Qt5Quick::getSize(int *pWidth, int *pHeight)
{
    QMutexLocker locker(&mMutex);

    if (pWidth)  *pWidth  = mWidth;
    if (pHeight) *pHeight = mHeight;
}

void Qt5Quick::getSizeForRenderer(int *pWidth, int *pHeight)
{
    QMutexLocker locker(&mMutex);
    
    if (pWidth)  *pWidth  = mWidth;
    if (pHeight) *pHeight = mHeight;
}

void Qt5Quick::getDisplayDPI(float *hdpi, float *vdpi, float *ddpi)
{
    QMutexLocker locker(&mMutex);

    if (hdpi != nullptr) *hdpi = mDpiHoriz;
    if (vdpi != nullptr) *vdpi = mDpiVert;
    if (ddpi != nullptr) *ddpi = mDpiDiag;
}

void Qt5Quick::create()
{
}

void Qt5Quick::show()
{
}

void Qt5Quick::hide()
{
}

void Qt5Quick::close()
{
}

bool Qt5Quick::isVisible() const
{
    return true;
}

void Qt5Quick::processEvents()
{
}

bool Qt5Quick::shouldQuit()
{
    return false;
}

bool Qt5Quick::shouldClose()
{
    return false;
}

bool Qt5Quick::sizeChanged()
{
    return mSizeChanged;
}

bool Qt5Quick::isKeyPressed(uint32_t key)
{
    return false;
}

bool Qt5Quick::isKeyPressedOnce(uint32_t key)
{
    return false;
}

bool Qt5Quick::isMousePressed(uint32_t button)
{
    return mMouseDown;
}

bool Qt5Quick::isMousePressedOnce(uint32_t button)
{
    return false;
}

std::pair<int, int> Qt5Quick::getMousePosition()
{
    return {mMouseX, mMouseY};
}

std::pair<float, float> Qt5Quick::getSwipeMovement()
{
    return {0,0};
}

bool Qt5Quick::isTouchPressed()
{
    return false;
}

void Qt5Quick::qtSetWidth(int width)
{
    QMutexLocker locker(&mMutex);

    mWidth = width;
    mSizeChanged = true;
}

void Qt5Quick::qtSetHeight(int height)
{
    QMutexLocker locker(&mMutex);

    mHeight = height;
    mSizeChanged = true;
}

void Qt5Quick::qtSetDpi(QScreen *screen)
{
    QMutexLocker locker(&mMutex);

    qreal hdpi = screen->physicalDotsPerInchX();
    qreal vdpi = screen->physicalDotsPerInchY();

    mDpiHoriz = hdpi;
    mDpiVert = vdpi;
    mDpiDiag = sqrt(2 * hdpi * vdpi);
}

bool Qt5Quick::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            emit requestClose();
            return true;
        }
        else if (keyEvent->key() == Qt::Key_P) {
            emit togglePause();
            return true;
        }
    }

    if (obj->objectName() == "IfCanvas") {
        if (event->type() == QEvent::MouseButtonPress) {
            QMutexLocker locker(&mMutex);
            mMouseDown = true;
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease) {
            QMutexLocker locker(&mMutex);
            mMouseDown = false;
            return true;
        }
        else if (event->type() == QEvent::MouseMove) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            QMutexLocker locker(&mMutex);
            mMouseX = mouseEvent->pos().x();
            mMouseY = mouseEvent->pos().y();
            return true;
        }
        else if (event->type() == QEvent::HoverMove) {
            auto hoverEvent = static_cast<QHoverEvent *>(event);
            QMutexLocker locker(&mMutex);
            mMouseX = hoverEvent->pos().x();
            mMouseY = hoverEvent->pos().y();
            return true;
        }
    }
    return obj->eventFilter(obj, event);
}

