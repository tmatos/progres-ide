#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingsdialog.h"
#include "finddialog.h"
#include "veriloghighlighter.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QSettings>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setCentralWidget(ui->splitter);

    config = new QSettings("Tiago Matos", "Progres IDE");

    if(!config->contains("simulator"))
    {
        QMessageBox::information(this,
                                 tr("Simulator program is not defined"),
                                 tr("Please, select the simulator program in the next screen."));

        QString simPath = QFileDialog::getOpenFileName(this, tr("Select the simulator program"), "", tr("Executable files (*)"));
        if(!simPath.isEmpty()) {
            config->setValue("simulator", simPath);
        }
    }

    if(config->contains("recents/show")) {
        if(config->value("recents/show").toBool()) {
            QStringList recentsList = config->value("recents/list").toStringList();
            auto maxRecentFiles = config->value("recents/max").toInt();
            updateRecentFilesList(recentsList, maxRecentFiles);
        }
    }
    else {
        QStringList empty;
        config->setValue("recents/show", true);
        config->setValue("recents/max", 10);
        config->setValue("recents/list", empty);
    }

    ui->tabFiles->clear();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// update recent files menu to the string list contents
void MainWindow::updateRecentFilesList(const QStringList &recentsList, const int maxRecentFiles)
{
    ui->menu_Recent->clear();

    ui->menu_Recent->addAction( new QAction(tr("Clear"), this) );
    connect( ui->menu_Recent->actions().first(), SIGNAL(triggered()), this, SLOT(on_clearRecentFilesMenu()) );

    ui->menu_Recent->addSeparator();

    for(auto i = 0 ; i < maxRecentFiles && i < recentsList.count() ; i++)
    {
        QString path = recentsList[i];
        ui->menu_Recent->addAction( new QAction(path, this) );
        ui->menu_Recent->actions().at(i+1)->setData(path);
        connect( ui->menu_Recent->actions().at(i+1), SIGNAL(triggered()), this, SLOT(openRecentFile()) );
    }
}

void MainWindow::on_clearRecentFilesMenu()
{
    QStringList empty;
    updateRecentFilesList(empty, 0);
    config->setValue("recents/list", empty);
}

void MainWindow::openRecentFile()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if(action) {
        openVerilogFile(action->data().toString());
    }
}

void MainWindow::findText(const QString &txt)
{
    QPlainTextEdit *edt = (QPlainTextEdit*)ui->tabFiles->currentWidget();

    if(edt) {
        edt->find(txt);
    }
}

void MainWindow::newVerilogFile()
{
    auto lastTabIndex = ui->tabFiles->count();

    QPlainTextEdit *fileEdit = new QPlainTextEdit(0);

    QFont codeFont("Monospace");
    codeFont.setStyleHint(QFont::TypeWriter);
    fileEdit->setFont(codeFont);

    QString title(tr("new_circuit_") + QString::number(newVerilogFileCount) + ".v");
    newVerilogFileCount++;
    ui->tabFiles->addTab(fileEdit, title);
    ui->tabFiles->setCurrentIndex(lastTabIndex);

    FileStatus fs;
    fs.modified = true;
    fileStatusList.append(fs);

    VerilogHighlighter *hlg = new VerilogHighlighter(fileEdit->document());
    Q_UNUSED(hlg);
}

void MainWindow::openVerilogFileDialog()
{
    QString filePath;
    filePath = QFileDialog::getOpenFileName(this, tr("Open Verilog Code"), "", tr("Verilog files (*.v)"));
    if(!filePath.isEmpty()) {
        openVerilogFile(filePath);
    }
}

void MainWindow::openVerilogFile(QString filePath)
{
    QFile file(filePath);

    if(file.exists())
    {
        ui->consoleEdit->setPlainText(tr("Opening file: ") + filePath);

        file.open(QFile::ReadWrite | QFile::Text); // TODO: check

        if(config->value("recents/show").toBool())
        {
            QStringList recentsList = config->value("recents/list").toStringList();
            auto maxRecentFiles = config->value("recents/max").toInt();
            if(recentsList.count() >= maxRecentFiles) {
                recentsList.removeAt(0);
            }

            recentsList.append(filePath);

            config->setValue("recents/list", recentsList);

            //FIXME duplicate entries

            updateRecentFilesList(recentsList, maxRecentFiles);
        }

        QPlainTextEdit *fileEdit = new QPlainTextEdit(0);

        QFont codeFont("Monospace");
        codeFont.setStyleHint(QFont::TypeWriter);
        fileEdit->setFont(codeFont);

        QString title(filePath);
        ui->tabFiles->addTab(fileEdit, title);
        ui->tabFiles->setCurrentIndex(ui->tabFiles->count()-1);

        QString code = QString::fromUtf8(file.readAll());
        fileEdit->setPlainText(code);

        VerilogHighlighter *hlg = new VerilogHighlighter(fileEdit->document());
        Q_UNUSED(hlg);

        FileStatus fs;
        fs.modified = false;
        fs.path = filePath;
        fileStatusList.append(fs);
    }
    else {
        ui->consoleEdit->appendPlainText(tr("Error loading file. Not found."));
    }
}

void MainWindow::saveVerilogFileDialog()
{
    auto i = ui->tabFiles->currentIndex();

}

void MainWindow::saveVerilogFileAsDialog()
{
    QString filePath;
    filePath = QFileDialog::getSaveFileName(this, tr("Open Verilog Code"), "", tr("Verilog files (*.v)"));

    // TODO
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dlg;
    dlg.setModal(true);
    dlg.exec();
}

void MainWindow::showAboutDialog()
{
    QString msg("Progress IDE\n\n(C) 2014-2022 Tiago Matos\n\nhttps://github.com/tmatos/progres\n");
    QMessageBox::about(this, "Progress IDE", msg);
}

void MainWindow::exitApp()
{
    close();
}

void MainWindow::simulationStart()
{
    unsigned int n = ui->tabFiles->count();

    if(!n) {
        return;
    }

    if(config->value("simulator").toString().isEmpty()) {
        QMessageBox::warning(this, tr("Simulator not set"), tr("The path to the simulator executable is not set."));
        showSettingsDialog();
        return;
    }

    // FIXME

    QProcess *process = new QProcess(this);
    QString simulator = config->value("simulator").toString();
    QString dir = ".";

    QString file = ui->tabFiles->tabText(ui->tabFiles->currentIndex());

    QStringList args;
    args << dir;
    args << file;

    process->start(simulator, args);

    ui->consoleEdit->appendPlainText("DEBUG: " + args[0] + "&" + args[1]);
}

void MainWindow::on_tabFiles_tabCloseRequested(int index)
{
    // TODO: checks

    // TODO: cleanup

    FileStatus fs = fileStatusList[index];

    if(fs.modified) {
        QMessageBox box;
        box.setWindowTitle(tr("File modified"));
        box.setText(tr("Save?"));
        box.setStandardButtons( QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel );
        box.setDefaultButton(QMessageBox::Cancel);

        auto response = box.exec();

        if(response == QMessageBox::Save) {
            // TODO
        }
        else if(response == QMessageBox::Cancel) {
            return;
        }

        // TODO: discard
    }

    fileStatusList.removeAt(index);

    // TODO: delete highlighter

    delete ui->tabFiles->widget(index); // NOTE: removeTab() doesn't free memory
}

void MainWindow::on_tabFiles_tabBarClicked(int index)
{
    ui->consoleEdit->appendPlainText("DEBUG: click at tab #" + QString::number(index));
}

void MainWindow::on_actionSelect_All_triggered()
{
    ( (QPlainTextEdit*)ui->tabFiles->currentWidget() )->selectAll();
}

void MainWindow::on_actionFind_triggered()
{
    if (!findDialog) {
        findDialog = new FindDialog(this);
    }

    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void MainWindow::on_actionClose_triggered()
{
    auto index = ui->tabFiles->currentIndex();

    if(index >= 0) {
        on_tabFiles_tabCloseRequested(index);
    }
}
