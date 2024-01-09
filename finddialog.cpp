#include <QPlainTextEdit>

#include<mainwindow.h>

#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::on_btnFind_clicked()
{
    MainWindow* win = (MainWindow*)this->parent();

    win->findText(ui->edtText->text());
}
