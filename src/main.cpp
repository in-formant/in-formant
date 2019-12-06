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

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    appFont = QFont(loadFont(":/fonts/Roboto/Roboto-Regular.ttf"));
    appFont.setPixelSize(16);
    app.setFont(appFont);

    Analyser analyser;
    MainWindow mainWindow;

    return app.exec();
}