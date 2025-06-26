#ifndef ORDER_H
#define ORDER_H

#include <QWidget>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>
#include<QPair>
#include<QString>
#include<QMap>

namespace Ui {
class order;
}

class order : public QWidget
{
    Q_OBJECT

public:
    explicit order(QWidget *parent = nullptr);

    void click_back_button();

    void click_order_tracking();

    void receive_message();

    void show_orders();

    std::string message;

    QTcpSocket socket;

    static QMap<QString,QMap<QString,QPair<QString,QString>>> order_history;

    static int price;

    ~order();

signals:

    void click_back();

    void click_tracking();

    void click_server();

public slots:
    void on_back_button_clicked();

    void send_message();

    void on_order_tracking_button_clicked();

private:
    Ui::order *ui;
};

#endif // ORDER_H
