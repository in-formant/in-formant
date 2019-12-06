#include <QApplication>
#include <iostream>
#include "gui/qt/MainWindow.h"

QString loadFont(const QString & url)
{
    int id = QFontDatabase::addApplicationFont(url);
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);

    return family;
}

QFont appFont;

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);

    appFont = QFont(loadFont(":/fonts/Roboto/Roboto-Regular.ttf"));
    appFont.setPixelSize(16);
    app.setFont(appFont);

    Analyser analyser;
    MainWindow mainWindow;

    return app.exec();
}

#ifdef _WIN32

static inline char *wideToMulti(int codePage, const wchar_t *aw)
{
    const int required = WideCharToMultiByte(codePage, 0, aw, -1, NULL, 0, NULL, NULL);
    char *result = new char[required];
    WideCharToMultiByte(codePage, 0, aw, -1, result, required, NULL, NULL);
    return result;
}

int WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdParamarg*/, int /* cmdShow */)
{
    int argc;
    wchar_t **argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW)
        return -1;
    char **argv = new char *[argc + 1];
    for (int i = 0; i < argc; ++i)
        argv[i] = wideToMulti(CP_ACP, argvW[i]);
    argv[argc] = nullptr;
    LocalFree(argvW);
    const int exitCode = main(argc, argv);
    for (int i = 0; i < argc && argv[i]; ++i)
        delete [] argv[i];
    delete [] argv;
    return exitCode;
}

#endif