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
    ~forgot_password();

signals:
    void click_back();
    void click_forgot();

public slots:
    void on_back_button_2_clicked();
    void on_forgot_password_button_clicked();

private:
    Ui::forgot_password *ui;
};

#endif // FORGOT_PASSWORD_H
