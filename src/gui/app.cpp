#include "gui.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickView>

QMutex mutex;

std::unique_ptr<Main::ContextManager> pManager;
QMutex *pManagerMutex = &mutex;

int Gui::runApp(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("InFormant");

    QQuickStyle::setStyle("Material");
    qmlRegisterType<Gui::Canvas>("IfCanvas", 1, 0, "IfCanvas");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    engine.load(QUrl("qrc:/main.qml"));
    
    static_cast<QQuickView *>(engine.rootObjects().first())->setResizeMode(QQuickView::SizeRootObjectToView);

    pManagerMutex->lock();
    pManager->initialize();
    pManagerMutex->unlock();

    return app.exec();
}
