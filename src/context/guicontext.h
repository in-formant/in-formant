#ifndef MAIN_GUI_CONTEXT_H
#define MAIN_GUI_CONTEXT_H

#include <memory>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QQuickView>
#include <QScreen>
#include <QTimer>

#include "synthwrapper.h"
#include "dataviswrapper.h"

namespace Main {

    class RenderContext;
    class Config;
    class DataStore;
    class GuiView;

    class GuiContext : public QObject {
        Q_OBJECT

    public:
#ifndef WITHOUT_SYNTH
        GuiContext(Config *config, RenderContext *renderContext, SynthWrapper *synthWrapper, DataVisWrapper *dataVisWrapper);
#else
        GuiContext(Config *config, RenderContext *renderContext, DataVisWrapper *dataVisWrapper);
#endif

        int exec();
    
        void setView(GuiView *view);

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    public slots:
        void setShowSpectrogram(bool b);
        void setShowPitch(bool b);
        void setShowFormants(bool b);

    private:
        void setWidth(int width);
        void setHeight(int height);
        void updateDpi(QScreen *screen);

        Config *mConfig;
        RenderContext *mRenderContext;

        GuiView *mSelectedView;

        std::unique_ptr<QApplication> mApp;
        std::unique_ptr<QQmlApplicationEngine> mQmlEngine;

        friend class GuiView;
    };

    class GuiView {
    protected:
        virtual bool onKeyPress(QKeyEvent *) { return false; }
        virtual bool onKeyRelease(QKeyEvent *) { return false; }

        friend bool GuiContext::eventFilter(QObject *obj, QEvent *event);
    };

}

#endif // MAIN_GUI_CONTEXT_H
