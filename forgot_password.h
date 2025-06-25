#ifndef FORGOT_PASSWORD_H
#define FORGOT_PASSWORD_H

#include <QWidget>
#include<QPushButton>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>

namespace Ui {
class forgot_password;
}

class forgot_password : public QWidget
{
    Q_OBJECT

public:
    explicit forgot_password(QWidget *parent = nullptr);

    void click_back_button();

    void click_forgot_password_button();

    void receive_message();

    std::string message;

    QTcpSocket socket;

    ~forgot_password();

signals:

    void click_back();

    void click_forgot();

    void click_server();

public slots:

    void on_back_button_2_clicked();

    void on_forgot_password_button_clicked();

    void send_message();

private:
    Ui::forgot_password *ui;

    QPushButton *back_button;

};

#endif // FORGOT_PASSWORD_H
