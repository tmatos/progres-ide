#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

protected slots:
    void accept();
    void selectSimulatorExe();

private:
    Ui::SettingsDialog *ui;
    QSettings* config;
};

#endif // SETTINGSDIALOG_H
