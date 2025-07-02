#include "sign_in.h"
#include "ui_sign_in.h"
#include "mainwindow.h"
#include "network_manager.h"
#include <QString>
#include <QFile>
#include <QMessageBox>

sign_in::sign_in(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::sign_in)
{
    ui->setupUi(this);

    // Connect network manager signals
    NetworkManager* netManager = NetworkManager::getInstance();
    connect(netManager, &NetworkManager::registerSuccess, this, &sign_in::onRegisterSuccess);
    connect(netManager, &NetworkManager::registerFailed, this, &sign_in::onRegisterFailed);
    connect(netManager, &NetworkManager::networkError, this, &sign_in::onNetworkError);

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
    QString confirm_password_line = ui->confirm_password_lineedit->text();
    QString userType;
    if(ui->manager_radiobutton && ui->manager_radiobutton->isChecked()) {
        userType = "manager";
    } else if(ui->customer_radiobutton && ui->customer_radiobutton->isChecked()) {
        userType = "customer";
    } else if(ui->restaurant_radiobutton && ui->restaurant_radiobutton->isChecked()) {
        userType = "restaurant";
    } else {
        QMessageBox::warning(this, "Input Error", "Please select a user type");
        return;
    }

    if (username_line.isEmpty() || password_line.isEmpty() || confirm_password_line.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter username, password, and confirm password");
        return;
    }

    if (password_line != confirm_password_line) {
        QMessageBox::warning(this, "Input Error", "Passwords do not match");
        return;
    }

    // Use network manager for registration
    NetworkManager* netManager = NetworkManager::getInstance();
    netManager->registerUser(username_line, password_line, userType);
}

void sign_in::onRegisterSuccess(const QString &message)
{
    QMessageBox::information(this, "Success", message);
    // Optionally, go back to login page or auto-login
    on_back_button_clicked();
}

void sign_in::onRegisterFailed(const QString &error)
{
    QMessageBox::warning(this, "Registration Failed", error);
}

void sign_in::onNetworkError(const QString &error)
{
    QMessageBox::critical(this, "Network Error", 
                         QString("Network error: %1\n\nPlease check if the server is running.").arg(error));
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
