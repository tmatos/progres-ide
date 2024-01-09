#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    config = new QSettings("Tiago Matos", "Progres IDE");

    QString simPath = config->value("simulator").toString();
    ui->edtSimulatorPath->setText(simPath);

    ui->chkShowRecent->setChecked(config->value("recents/show").toBool());
    ui->boxMaxRecent->setValue(config->value("recents/max").toInt());
}

SettingsDialog::~SettingsDialog()
{
    delete config;
    delete ui;
}

void SettingsDialog::accept()
{
    // TODO: validation

    config->setValue("simulator", ui->edtSimulatorPath->text());
    config->setValue("recents/show", ui->chkShowRecent->isChecked());
    config->setValue("recents/max", ui->boxMaxRecent->value());

    close();
}
