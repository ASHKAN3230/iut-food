#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sign_in.h"
#include "forgot_password.h"
#include "customer.h"
#include<QString>
#include<QFile>
#include<QTextStream>
#include<QStringList>
#include<QMessageBox>

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

    QString username_line;

    QString password_line;

    username_line = ui->username_lineedit->text();

    password_line = ui->password_lineedit->text();

    bool state = false;

    QFile file_1(QString(SRC_PATH) + "files/system_manage_list.txt");

    if(file_1.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_1);

        QStringList strlist;

        while(!in.atEnd())
        {

            strlist.append(in.readLine());

        }

        file_1.close();

        for(int i = 0; i < strlist.size() - 1; i += 2)
        {

            const QString &line_1 = strlist.at(i);

            const QString &line_2 = strlist.at(i+1);

            if(username_line == line_1 && password_line == line_2)
            {

                state = true;

                QMessageBox::information(this,"success","username and password are correct");//change

                //system_manager

            }

        }

    }

    QFile file_2(QString(SRC_PATH) + "files/restaurant_managers_list.txt");

    if(file_2.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_2);

        QStringList strlist;

        while(!in.atEnd())
        {

            strlist.append(in.readLine());

        }

        file_2.close();

        for(int i = 0; i < strlist.size() - 1; i += 2)
        {

            const QString &line_1 = strlist.at(i);

            const QString &line_2 = strlist.at(i+1);

            if(username_line == line_1 && password_line == line_2)
            {

                state = true;

                QMessageBox::information(this,"success","username and password are correct");//change

                // restaurant_manager

            }

        }
    }

    QFile file_3(QString(SRC_PATH) + "files/customers_list.txt");

    if(file_3.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_3);

        QStringList strlist;

        while(!in.atEnd())
        {

            strlist.append(in.readLine());

        }

        file_3.close();

        for(int i = 0; i < strlist.size() - 1; i += 2)
        {

            const QString &line_1 = strlist.at(i);

            const QString &line_2 = strlist.at(i+1);

            if(username_line == line_1 && password_line == line_2)
            {

                state = true;

                customer *person = new customer();

                person->setAttribute(Qt::WA_DeleteOnClose);

                person->show();

                this->close();

            }

        }
    }

    if(state == false)
    {

        QMessageBox::warning(this,"fail","username or password is not correct");

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
