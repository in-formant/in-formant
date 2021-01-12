#ifndef TARGET_QT5QUICK_H
#define TARGET_QT5QUICK_H

#include "../base/base.h"

#include <QQuickFramebufferObject>
#include <QMutex>

namespace Module::Target {

    class Qt5Quick : public QObject, public AbstractBase {
        Q_OBJECT

    public:
        Qt5Quick(Type rendererType);
        virtual ~Qt5Quick();

        void initialize() override;
        void terminate() override;

        void setTitle(const std::string& title) override;
        void setSize(int width, int height) override;

        void getSize(int *pWidth, int *pHeight) override;
        void getSizeForRenderer(int *pWidth, int *pHeight) override;
        void getDisplayDPI(float *hdpi, float *vdpi, float *ddpi) override;

        void create() override;
        void show() override;
        void hide() override;
        void close() override;

        bool isVisible() const override;

        void processEvents() override;
        bool shouldQuit() override;
        bool shouldClose() override;
        bool sizeChanged() override;
        bool isKeyPressed(uint32_t key) override;
        bool isKeyPressedOnce(uint32_t key) override;
        bool isMousePressed(uint32_t key) override;
        bool isMousePressedOnce(uint32_t key) override;
        std::pair<int, int> getMousePosition() override;

        std::pair<float, float> getSwipeMovement() override;
   
        bool isTouchPressed() override;
    
    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;
            
    public slots:
        void qtSetWidth(int width);
        void qtSetHeight(int height);
        void qtSetDpi(QScreen *screen);

    signals:
        void requestClose();
        void togglePause();
        void displayLpSpec(bool);
        void useFrameCursor(bool);
        void displayFormantTracks(bool);
        void displayPitchTracks(bool);

    private:
        QMutex mMutex;

        int mWidth, mHeight;
        float mDpiHoriz, mDpiVert, mDpiDiag;
        bool mMouseDown;
        int mMouseX, mMouseY;
        bool mSizeChanged;
    };

#ifdef RENDERER_USE_NVG
    class Q5Q_NanoVG : public NvgProvider {
    public:
        Q5Q_NanoVG();

        NVGcontext *createContext(int flags) override;
        void deleteContext(NVGcontext *ctx) override;
       
        void beforeBeginFrame() override; 
        void afterEndFrame() override;
    
        void *createFramebuffer(NVGcontext *ctx, int width, int height, int imageFlags) override;
        void bindFramebuffer(void *framebuffer) override;
        void deleteFramebuffer(void *framebuffer) override;
        int framebufferImage(void *framebuffer) override;

    private:
        void createGLContext();
        void destroyGLContext();
    };
#endif

}

#endif // TARGET_QT5QUICK_H
