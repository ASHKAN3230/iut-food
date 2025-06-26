#ifndef RATE_H
#define RATE_H

#include <QWidget>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>
#include<QPair>
#include<QMap>

namespace Ui {
class rate;
}

class rate : public QWidget
{
    Q_OBJECT

public:
    explicit rate(QWidget *parent = nullptr);

    void click_back_button();

    void click_save_button();

    void receive_message();

    void show_rates();

    QMap<int,QPair<QString,QPair<QString,QString>>> rate_information;

    static int order_count;

    static int index;

    std::string message;

    QTcpSocket socket;

    ~rate();

signals:

    void click_back();

    void click_save();

    void click_server();

public slots:
    void on_back_button_clicked();

    void on_save_button_clicked();

    void send_message();

private:
    Ui::rate *ui;
};

#endif // RATE_H
