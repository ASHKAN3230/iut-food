#include "sign_in.h"
#include "ui_sign_in.h"
#include "mainwindow.h"
#include<QString>
#include<QFile>
#include<QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

sign_in::sign_in(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::sign_in)
{
    ui->setupUi(this);

    back_button = new QPushButton();

    connect(this,&sign_in::click_on_back_button,this,&sign_in::send_message);

    connect(this,&sign_in::receive_message,this,&sign_in::on_back_button_clicked);

    connect(this,&sign_in::click_on_sign_button,this,&sign_in::send_message);

    connect(this,&sign_in::receive_message,this,&sign_in::on_sign_in_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

}

sign_in::~sign_in()
{
    delete ui;
}

void sign_in::click_on_back_button()
{

    message = "back";

    emit click_server();

}

void sign_in::on_back_button_clicked()
{

    MainWindow *mw = new MainWindow();

    mw->setAttribute(Qt::WA_DeleteOnClose);

    mw->show();

    this->close();

}

void sign_in::click_on_sign_button()
{

    message = "signin";

    emit click_server();

}


void sign_in::on_sign_in_button_clicked()
{
    QString username_line = ui->username_lineedit->text();
    QString password_line = ui->password_lineedit->text();
    QString userType;
    if(ui->manager_radiobutton->isChecked()) {
        userType = "manager";
    } else if(ui->customer_radiobutton->isChecked()) {
        userType = "customer";
    } else if(ui->restaurant_radiobutton && ui->restaurant_radiobutton->isChecked()) {
        userType = "restaurant";
    } else {
        QMessageBox::warning(this, "fail", "Please select a user type");
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("iut_food.db");
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", db.lastError().text());
        return;
    }
    // Create users table if it doesn't exist
    QSqlQuery createQuery;
    createQuery.exec("CREATE TABLE IF NOT EXISTS users ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "username TEXT NOT NULL UNIQUE,"
                    "password TEXT NOT NULL,"
                    "user_type TEXT NOT NULL CHECK(user_type IN ('customer', 'manager', 'restaurant'))"
                    ");");

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, user_type) VALUES (?, ?, ?)");
    query.addBindValue(username_line);
    query.addBindValue(password_line);
    query.addBindValue(userType);
    if (query.exec()) {
        QMessageBox::information(this, "success", "Sign in is successful");
        // Optionally, open the relevant page here
    } else {
        QMessageBox::warning(this, "fail", "Sign in failed: " + query.lastError().text());
    }
}

void sign_in::send_message()
{

    if(socket.state() == QTcpSocket::ConnectedState)
    {

        QByteArray byte(this->message.c_str(),this->message.length());

        if(socket.write(byte) > 0)
        {

            if(!socket.waitForBytesWritten(1000))
            {

                QMessageBox::information(this,"not success","message not send!!!");

            }

        }

    }

}

void sign_in::receive_message()
{

    if(socket.state() == QTcpSocket::ConnectedState)
    {

        if(socket.waitForReadyRead(-1))
        {

            QByteArray byte = socket.readAll();

            this->message = std::string(byte.constData(),byte.length());

            if(message == "start back")
            {

                message = "";

                emit click_back();

            }
            else if(message == "start signin")
            {

                message = "";

                emit click_sign();

            }

        }

    }

}
