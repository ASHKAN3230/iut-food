#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sign_in.h"
#include "forgot_password.h"
#include "customer.h"
#include "menu_restaurant.h"
#include "restaurant_auth.h"
#include "network_manager.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect network manager signals
    NetworkManager* netManager = NetworkManager::getInstance();
    connect(netManager, &NetworkManager::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(netManager, &NetworkManager::loginFailed, this, &MainWindow::onLoginFailed);
    connect(netManager, &NetworkManager::networkError, this, &MainWindow::onNetworkError);

    // Check server health on startup
    netManager->checkServerHealth();

    // Replace the existing QLabel widgets with clicklabel widgets
    // Store the original geometry and style
    QRect signinGeometry = ui->signin_label->geometry();
    QString signinStyle = ui->signin_label->styleSheet();
    QString signinText = ui->signin_label->text();
    
    QRect forgotGeometry = ui->forgot_password_label->geometry();
    QString forgotStyle = ui->forgot_password_label->styleSheet();
    QString forgotText = ui->forgot_password_label->text();
    
    // Get the parent widget of the original labels
    QWidget* parentWidget = ui->signin_label->parentWidget();
    
    // Create clicklabel widgets
    clicklabel *signin_label = new clicklabel(parentWidget);
    signin_label->setText(signinText);
    signin_label->setGeometry(signinGeometry);
    signin_label->setStyleSheet(signinStyle);
    
    clicklabel *forgot_password_label = new clicklabel(parentWidget);
    forgot_password_label->setText(forgotText);
    forgot_password_label->setGeometry(forgotGeometry);
    forgot_password_label->setStyleSheet(forgotStyle);
    
    // Hide the original QLabel widgets
    ui->signin_label->hide();
    ui->forgot_password_label->hide();

    // Connect the clicklabel signals
    QObject::connect(signin_label, &clicklabel::clicked, this, &MainWindow::open_signin_window);
    QObject::connect(forgot_password_label, &clicklabel::clicked, this, &MainWindow::open_forgot_window);

    // Connect login button
    QObject::connect(ui->login_button, &QPushButton::clicked, this, &MainWindow::on_login_button_clicked);

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
    
    if (username_line.isEmpty() || password_line.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both username and password");
        return;
    }
    
    // Use network manager for login
    NetworkManager* netManager = NetworkManager::getInstance();
    netManager->login(username_line, password_line);
}

void MainWindow::onLoginSuccess(const QJsonObject &userInfo)
{
    QString username = userInfo["username"].toString();
    QString userType = userInfo["userType"].toString();
    int userId = userInfo["id"].toInt();
    
    qDebug() << "Login successful for user:" << username << "Type:" << userType;
    
    if (userType == "customer") {
        customer *person = new customer(username);
        person->setAttribute(Qt::WA_DeleteOnClose);
        person->showMaximized();
        this->close();
    } else if (userType == "manager") {
        menu_restaurant *mr = new menu_restaurant(username, -1);
        mr->setAttribute(Qt::WA_DeleteOnClose);
        mr->showMaximized();
        this->close();
    } else if (userType == "restaurant") {
        int restaurantId = userInfo["restaurantId"].toInt();
        if (restaurantId > 0) {
            // Restaurant has setup, go to menu management
            menu_restaurant *mr = new menu_restaurant(username, restaurantId);
            mr->setAttribute(Qt::WA_DeleteOnClose);
            mr->showMaximized();
            this->close();
        } else {
            // Restaurant needs setup
            restaurant_auth *ra = new restaurant_auth(username);
            ra->setAttribute(Qt::WA_DeleteOnClose);
            ra->showMaximized();
            this->close();
        }
    }
}

void MainWindow::onLoginFailed(const QString &error)
{
    QMessageBox::warning(this, "Login Failed", error);
}

void MainWindow::onNetworkError(const QString &error)
{
    QMessageBox::critical(this, "Network Error", 
                         QString("Network error: %1\n\nPlease check if the server is running.").arg(error));
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
