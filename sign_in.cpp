#include "sign_in.h"
#include "ui_sign_in.h"
#include "mainwindow.h"
#include<QString>
#include<QFile>
#include<QMessageBox>

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

    QString username_line;

    QString password_line;

    username_line = ui->username_lineedit->text();

    password_line = ui->password_lineedit->text();

    if(ui->manager_radiobutton->isChecked())
    {

        QFile file_3("C:/Users/Asus/Desktop/customer-section/untitled/files/restaurant_managers_list.txt");

        if(file_3.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        {

            QTextStream out(&file_3);

            out << username_line << "\n";

            out << password_line << "\n";

            file_3.close();

        }

        QMessageBox::information(this,"success","sgin in is successful");

    }
    else if(ui->customer_radiobutton->isChecked())
    {

        QFile file_4("C:/Users/Asus/Desktop/customer-section/untitled/files/customers_list.txt");

        if(file_4.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        {

            QTextStream out(&file_4);

            out << username_line << "\n";

            out << password_line << "\n";

            file_4.close();

        }

        QMessageBox::information(this,"success","sgin in is successful");

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

