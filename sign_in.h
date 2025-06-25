#ifndef SIGN_IN_H
#define SIGN_IN_H

#include <QWidget>
#include<QPushButton>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>

namespace Ui {
class sign_in;
}

class sign_in : public QWidget
{
    Q_OBJECT

public:
    explicit sign_in(QWidget *parent = nullptr);

    void click_on_back_button();

    void click_on_sign_button();

    void receive_message();

    QTcpSocket socket;

    std::string message;

    ~sign_in();

signals:

    void click_back();

    void click_sign();

    void click_server();

public slots:

    void on_back_button_clicked();

    void on_sign_in_button_clicked();

    void send_message();

private:
    Ui::sign_in *ui;

    QPushButton *back_button;

};

#endif // SIGN_IN_H

