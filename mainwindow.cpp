#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sign_in.h"
#include "forgot_password.h"
#include "customer.h"
#include "menu_restaurant.h"
#include "restaurant_auth.h"
#include<QString>
#include<QFile>
#include<QTextStream>
#include<QStringList>
#include<QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    signin_label = new clicklabel(this);

    signin_label->setText(ui->signin_label->text());

    signin_label->setGeometry(880,540,291,16);

    signin_label->setStyleSheet(ui->signin_label->styleSheet());

    signin_label->setParent(ui->centralwidget);

    forgot_password_label = new clicklabel(this);

    forgot_password_label->setText(ui->forgot_password_label->text());

    forgot_password_label->setGeometry(880,520,291,16);

    forgot_password_label->setStyleSheet(ui->forgot_password_label->styleSheet());

    forgot_password_label->setParent(ui->centralwidget);

    QObject::connect(signin_label,&clicklabel::clicked,this,&MainWindow::open_signin_window);

    QObject::connect(forgot_password_label,&clicklabel::clicked,this,&MainWindow::open_forgot_window);

    QObject::connect(this, &MainWindow::click_login_button, this, &MainWindow::send_message);

    QObject::connect(this, &MainWindow::receive_message, this, &MainWindow::on_login_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

}

void MainWindow::open_signin_window()
{

    sign_in *signin = new sign_in();

    signin->setAttribute(Qt::WA_DeleteOnClose);

    signin->show();

    this->close();

}

void MainWindow::open_forgot_window()
{

    forgot_password *forgotpassword = new forgot_password();

    forgotpassword->setAttribute(Qt::WA_DeleteOnClose);

    forgotpassword->show();

    this->close();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::click_login_button()
{

    message = "login";

    emit click_server();

}

void MainWindow::on_login_button_clicked()
{
    QString username_line = ui->username_lineedit->text();
    QString password_line = ui->password_lineedit->text();
    bool state = false;

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
    query.prepare("SELECT user_type FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username_line);
    query.addBindValue(password_line);
    if (query.exec() && query.next()) {
        QString userType = query.value(0).toString();
        state = true;
        if (userType == "customer") {
            customer *person = new customer();
            person->setAttribute(Qt::WA_DeleteOnClose);
            person->showMaximized();
            this->close();
            return;
        } else if (userType == "manager") {
            menu_restaurant *mr = new menu_restaurant();
            mr->setAttribute(Qt::WA_DeleteOnClose);
            mr->showMaximized();
            this->close();
            return;
        } else if (userType == "restaurant") {
            // Redirect restaurant users to authentication page
            restaurant_auth *ra = new restaurant_auth();
            ra->setAttribute(Qt::WA_DeleteOnClose);
            ra->showMaximized();
            this->close();
            return;
        }
    }
    if (!state) {
        QMessageBox::warning(this, "fail", "username or password is not correct");
    }
}

void MainWindow::send_message()
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

void MainWindow::receive_message()
{

        if(socket.state() == QTcpSocket::ConnectedState)
        {

            if(socket.waitForReadyRead(-1))
            {

                QByteArray byte = socket.readAll();

                this->message = std::string(byte.constData(),byte.length());

                if(message == "start login")
                {

                    message = "";

                    emit click();

                }

            }

        }

}
