#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "settingsdialog.h"
#include "finddialog.h"
#include "veriloghighlighter.h"

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QTextStream>
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

        QString simPath = QFileDialog::getOpenFileName(this,
                                                       tr("Select the simulator program"),
                                                       "",
                                                       tr("Executable files (*)"));
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

    // below: the signal is not being found
    //connect( ui->tabFiles->tabBar(), SIGNAL(tabMoved(int from, int to)), this, SLOT(on_tabMoved(int,int)) );

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

void MainWindow::on_fileEdit_textChanged()
{
    auto index = ui->tabFiles->currentIndex();
    auto tabTitle = ui->tabFiles->tabText(index);

    for (FileStatus &it : fileStatusList) {
        if(it.path == tabTitle) {
            it.modified = true;
            break;
        }
    }

    // TODO subclass the widget, in near future
}

void MainWindow::on_tabMoved(int from, int to)
{
    ui->consoleEdit->appendPlainText(tr("DEBUG: tab moved from ") + QString::number(from) + " to " + QString::number(to));

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
    fs.isNew = true;
    fs.modified = true;
    fs.path = title;
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

    if(!file.exists()) {
        ui->consoleEdit->appendPlainText(tr("Error loading file. Not found."));
        return;
    }

    ui->consoleEdit->appendPlainText(tr("Opening file: ") + filePath);

    auto success = file.open(QFile::ReadWrite | QFile::Text);
    if(!success) {
        ui->consoleEdit->appendPlainText(tr("Error loading file."));
        return;
    }

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
    fs.isNew = false;
    fs.modified = false;
    fs.path = filePath;
    fileStatusList.append(fs);

    connect( fileEdit, SIGNAL(textChanged()), this, SLOT(on_fileEdit_textChanged()) );
}

void MainWindow::on_actionSave_triggered()
{
    if(ui->tabFiles->count() == 0) {
        return;
    }

    auto index = ui->tabFiles->currentIndex();

    ui->consoleEdit->appendPlainText("DEBUG: saving file on tab " + QString::number(index));

    QString tabTitle = ui->tabFiles->tabText(index);
    FileStatus &fs = fileStatusList.first();

    for (FileStatus &it : fileStatusList) {
        if(it.path == tabTitle) {
            fs = it;
            break;
        }
    }

    if(fs.modified) {
        if(fs.isNew) {
            saveNewFile(fs, index);
        }
        else {
            saveExistingFile(fs, index);
        }
    }
}

void MainWindow::on_actionSave_As_triggered()
{
    if(ui->tabFiles->count() == 0) {
        return;
    }

    auto i = ui->tabFiles->currentIndex();

    QString filePath;
    filePath = QFileDialog::getSaveFileName(this, tr("Save Verilog File As"), "", tr("Verilog files (*.v)"));

    QFile file(filePath);

    if(file.exists()) {
        // TODO
    }
    else {
        // TODO
    }

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
    auto n = ui->tabFiles->count();

    if(!n) {
        return;
    }

    if(config->value("simulator").toString().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Simulator not set"),
                             tr("The path to the simulator executable is not set."));
        showSettingsDialog();
        return;
    }

    // FIXME

    QString simulatorPath = config->value("simulator").toString();
    QFile simulatorExe(simulatorPath);

    if(!simulatorExe.exists()) {
        QMessageBox::warning(this,
                             tr("Fatal error"),
                             tr("Could not find the simulator: \'") + simulatorPath + "\'.");
        return;
    }

    QProcess *process = new QProcess(this);
    QString dir = ".";

    QString file = ui->tabFiles->tabText(ui->tabFiles->currentIndex());

    QStringList args;
    //args << dir;
    args << file;

    // TODO pass input files

    ui->consoleEdit->appendPlainText("EXECUTANDO: " + simulatorPath + " " + args[0]);

    process->start(simulatorPath, args);
    process->waitForFinished();
    QString output(process->readAllStandardOutput());

    ui->consoleEdit->appendPlainText(output);
}

bool MainWindow::saveNewFile(MainWindow::FileStatus &fs, const int index)
{
    QString newFilePath;
    newFilePath = QFileDialog::getSaveFileName(this, tr("Saving a new file"), "", tr("Verilog files (*.v)"));
    if(newFilePath.isEmpty()) {
        ui->consoleEdit->appendPlainText(tr("Saving the file was canceled."));
        return false;
    }
    if( !newFilePath.endsWith(".v") ) {
        newFilePath += ".v";
        // TODO: check case
    }
    QFile file(newFilePath);
    if( file.open(QFile::WriteOnly | QFile::Text) ) {
        QTextStream out(&file);
        out << ( (QPlainTextEdit*) ui->tabFiles->widget(index) )->toPlainText();
        fs.isNew = false;
        fs.modified = false;
        fs.path = newFilePath;
        file.close();
        ui->tabFiles->setTabText(index, newFilePath);
        ui->consoleEdit->appendPlainText(tr("Success. New file was saved as \'") + newFilePath + "\'.");
    }
    else {
        ui->consoleEdit->appendPlainText(tr("Error! Unable to save the file \'") + newFilePath + "\'.");
        return false;
    }
    return true;
}

bool MainWindow::saveExistingFile(MainWindow::FileStatus &fs, const int index)
{
    QString filePath = fs.path;
    QFile file(filePath);

    if( !file.open(QFile::ReadWrite | QFile::Text) ) {
        ui->consoleEdit->appendPlainText(tr("Error! Unable to save the file \'") + filePath + "\'.");
        return false;
    }

    QTextStream out(&file);
    out << ( (QPlainTextEdit*) ui->tabFiles->widget(index) )->toPlainText();
    fs.modified = false;
    file.close();
    ui->consoleEdit->appendPlainText(tr("Success. File \'") + filePath + "\' was saved.");

    return true;
}

void MainWindow::on_tabFiles_tabCloseRequested(int index)
{
    QString tabTitle = ui->tabFiles->tabText(index);

    FileStatus &fs = fileStatusList.first();

    for (FileStatus &it : fileStatusList) {
        if(it.path == tabTitle) {
            fs = it;
            break;
        }
    }

    if(fs.modified) {
        QMessageBox box;
        box.setWindowTitle(tr("File modified"));
        box.setText(tr("Save?"));
        box.setStandardButtons( QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel );
        box.setDefaultButton(QMessageBox::Cancel);

        auto response = box.exec();

        if(response == QMessageBox::Save) {
            if(fs.isNew) {
                if( !saveNewFile(fs, index) ) {
                    return;
                }
                // TODO: add created file to recents on menu
            }
            else {
                saveExistingFile(fs, index);
            }
        }
        else if(response == QMessageBox::Cancel) {
            return;
        }
    }

    //fileStatusList.removeOne(fs);
    auto i = 0;
    for (const FileStatus it : fileStatusList) {
        if(it.path == tabTitle) {
            fileStatusList.removeAt(i);
            break;
        }
        i++;
    }

    // TODO: delete highlighter (memleak?)

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
