#ifndef MENU_RESTAURANT_H
#define MENU_RESTAURANT_H

#include <QWidget>
#include<QString>
#include<QPair>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>

namespace Ui {
class menu_restaurant;
}

class menu_restaurant : public QWidget
{
    Q_OBJECT

public:
    explicit menu_restaurant(QWidget *parent = nullptr);

    void click_back_button();

    void click_shopping_basket_button();

    void open_menu_file();

    void receive_message();

    static int index;

    std::string message;

    QTcpSocket socket;

    QMap<QString,QMap<QString,QPair<QString,QString>>> menu_list;

    ~menu_restaurant();

signals:

    void click_back();

    void click_shop();

    void click_server();

public slots:
    void on_back_button_clicked();

    void on_shopping_basket_button_clicked();

    void send_message();

private:
    Ui::menu_restaurant *ui;
};

#endif // MENU_RESTAURANT_H

