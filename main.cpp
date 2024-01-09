#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/qrc/pt-BR.qm");

    a.installTranslator(&translator);

    MainWindow w;
    w.show();

    return a.exec();
}
