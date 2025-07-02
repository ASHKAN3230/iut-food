#include "forgot_password.h"
#include "ui_forgot_password.h"
#include "mainwindow.h"
#include<QString>
#include<QFile>
#include<QMessageBox>

forgot_password::forgot_password(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::forgot_password)
{
    ui->setupUi(this);
}

forgot_password::~forgot_password()
{
    delete ui;
}

void forgot_password::on_back_button_2_clicked()
{

    MainWindow *mw = new MainWindow();

    mw->setAttribute(Qt::WA_DeleteOnClose);

    mw->show();

    this->close();

}


void forgot_password::on_forgot_password_button_clicked()
{
    QString username = ui->username_lineedit->text();
    QString password = ui->password_lineedit->text();
    QString confirm_password = ui->confirm_password_lineedit->text();

    if (username.isEmpty() || password.isEmpty() || confirm_password.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields");
        return;
    }
    if (password != confirm_password) {
        QMessageBox::warning(this, "Input Error", "Passwords do not match");
        return;
    }
    // Simulate password reset
    QMessageBox::information(this, "Success", "Password has been reset!");
    on_back_button_2_clicked();
}
