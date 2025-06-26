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

    back_button = new QPushButton();

    connect(this,&forgot_password::click_back_button,this,&forgot_password::send_message);

    connect(this,&forgot_password::receive_message,this,&forgot_password::on_back_button_2_clicked);

    connect(this,&forgot_password::click_forgot_password_button,this,&forgot_password::send_message);

    connect(this,&forgot_password::receive_message,this,&forgot_password::on_forgot_password_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

}

forgot_password::~forgot_password()
{
    delete ui;
}

void forgot_password::click_back_button()
{

    message = "back";

    emit click_server();

}

void forgot_password::click_forgot_password_button()
{

    message = "forgot";

    emit click_server();

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

    QString username_line;

    username_line = ui->username_lineedit->text();

    bool count = false;

    QFile file_5("files/system_manage_list.txt");

    if(file_5.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_5);

        QStringList strlist;

        while(!in.atEnd())
        {

            strlist.append(in.readLine());

        }

        file_5.close();

        for(int i = 0; i < strlist.size() - 1; i += 2)
        {

            const QString &line_1 = strlist.at(i);

            const QString &line_2 = strlist.at(i+1);

            if(username_line == line_1)
            {

                count = true;

                ui->password_lineedit->setText(line_2);

            }

        }

    }

    QFile file_6("files/restaurant_managers_list.txt");

    if(file_6.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_6);

        QStringList strlist;

        while(!in.atEnd())
        {

            strlist.append(in.readLine());

        }

        file_6.close();

        for(int i = 0; i < strlist.size() - 1; i += 2)
        {

            const QString &line_1 = strlist.at(i);

            const QString &line_2 = strlist.at(i+1);

            if(username_line == line_1)
            {

                count = true;

                ui->password_lineedit->setText(line_2);

            }

        }
    }

    QFile file_7("files/customers_list.txt");

    if(file_7.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_7);

        QStringList strlist;

        while(!in.atEnd())
        {

            strlist.append(in.readLine());

        }

        file_7.close();

        for(int i = 0; i < strlist.size() - 1; i += 2)
        {

            const QString &line_1 = strlist.at(i);

            const QString &line_2 = strlist.at(i+1);

            if(username_line == line_1)
            {

                count = true;

                ui->password_lineedit->setText(line_2);

            }

        }
    }

    if(count == 0)
    {

        QMessageBox::warning(this,"fail","username or password is not correct");

    }

}

void forgot_password::send_message()
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

void forgot_password::receive_message()
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
            else if(message == "start forgot")
            {

                message = "";

                emit click_forgot();

            }

        }

    }

}
