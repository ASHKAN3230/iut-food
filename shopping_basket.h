#ifndef SHOPPING_BASKET_H
#define SHOPPING_BASKET_H

#include <QWidget>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>

namespace Ui {
class shopping_basket;
}

class shopping_basket : public QWidget
{
    Q_OBJECT

public:
    explicit shopping_basket(const QString &username, QWidget *parent = nullptr);

    void click_back_button();

    void click_save_order_button();

    static QMap<QString,QMap<QString,QPair<QString,QString>>> shop_basket;

    static int sum;

    void show_list();

    void receive_message();

    std::string message;

    QTcpSocket socket;

    ~shopping_basket();

    void set_lables();

    void set_items();

signals:

    void click_back();

    void click_server();

    void click_order();

public slots:
    void on_back_button_clicked();

    void send_message();

    void on_save_order_button_clicked();

    void on_order_button_clicked();

private:
    Ui::shopping_basket *ui;
    QString currentRestaurantUsername;
};

#endif // SHOPPING_BASKET_H
