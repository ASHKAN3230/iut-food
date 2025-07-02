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
    setFixedSize(400, 500);
    // Connect network manager signals
    NetworkManager* netManager = NetworkManager::getInstance();
    connect(netManager, &NetworkManager::loginSuccess, this, &MainWindow::onLoginSuccess);
    connect(netManager, &NetworkManager::loginFailed, this, &MainWindow::onLoginFailed);
    connect(netManager, &NetworkManager::networkError, this, &MainWindow::onNetworkError);
    // Check server health on startup
    netManager->checkServerHealth();
    // Connect the clickable labels for sign up and forgot password
    connect(ui->signin_label, &QLabel::linkActivated, this, &MainWindow::open_signin_window);
    connect(ui->forgot_password_label, &QLabel::linkActivated, this, &MainWindow::open_forgot_window);
    // Make labels clickable (simulate link)
    ui->signin_label->setText("<a href='#'>Sign up</a>");
    ui->signin_label->setTextFormat(Qt::RichText);
    ui->signin_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->signin_label->setOpenExternalLinks(false);
    ui->forgot_password_label->setText("<a href='#'>Forgot password?</a>");
    ui->forgot_password_label->setTextFormat(Qt::RichText);
    ui->forgot_password_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->forgot_password_label->setOpenExternalLinks(false);
    // Connect login button
    connect(ui->login_button, &QPushButton::clicked, this, &MainWindow::on_login_button_clicked);
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
        customer *person = new customer(username, userId);
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
