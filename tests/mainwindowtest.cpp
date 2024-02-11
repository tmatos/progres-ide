#include <QObject>
#include <QtTest/QtTest>

#include "../mainwindow.h"

class MainWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void test_showAboutDialog();

};

void MainWindowTest::test_showAboutDialog()
{
    MainWindow w;

    QTest::keyClick(&w, Qt::Key_F1);
}
