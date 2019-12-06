#include <QApplication>
#include <iostream>
#include "gui/qt/MainWindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Analyser analyser;
    MainWindow mainWindow;

    return app.exec();
}