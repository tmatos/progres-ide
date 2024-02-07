#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void updateRecentFilesList(const QStringList& recentsList, const int maxRecentFiles);
    void openVerilogFile(QString filePath);
    void findText(const QString &txt);

protected slots:
    void newVerilogFile();
    void openVerilogFileDialog();
    void saveVerilogFileDialog();
    void saveVerilogFileAsDialog();
    void showSettingsDialog();
    void showAboutDialog();
    void exitApp();
    void simulationStart();
    void openRecentFile();

private slots:
    void on_tabFiles_tabCloseRequested(int index);
    void on_tabFiles_tabBarClicked(int index);
    void on_clearRecentFilesMenu();

    void on_actionSelect_All_triggered();

    void on_actionFind_triggered();

    void on_actionClose_triggered();

private:
    Ui::MainWindow *ui;

    QSettings* config = nullptr;

    int maxRecentFiles = 10;
    QList<QAction> recentFilesActionList;

    unsigned int newVerilogFileCount = 0;

    struct FileStatus
    {
        FileStatus()
        {
            modified = false;
            path = QString("");
        }
        bool modified;
        QString path;
    };

    QList<FileStatus> fileStatusList;

    QDialog *findDialog = nullptr;
};

#endif // MAINWINDOW_H
