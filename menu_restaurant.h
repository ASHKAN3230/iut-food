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
    explicit menu_restaurant(const QString &username, QWidget *parent = nullptr);
    ~menu_restaurant();

    void click_back_button();

    void click_shopping_basket_button();

    void open_menu_from_database();

    void receive_message();

    void save_menu_to_database();

    void refresh_menu_display();

    void clear_form();

    void load_orders();

    void populate_orders_table();

    static int index;

    std::string message;

    QTcpSocket socket;

    QMap<QString,QMap<QString,QPair<QString,QString>>> menu_list;

signals:

    void click_shop();

    void click_server();

public slots:
    void on_shopping_basket_button_clicked();

    void send_message();

    void on_addFoodButton_clicked();

    void on_editFoodButton_clicked();

    void on_deleteFoodButton_clicked();

    void on_clearFormButton_clicked();

    void on_menuItem_selected();

    void on_profile_button_clicked();

    void on_logout_button_clicked();

    void on_refreshOrdersButton_clicked();

    void on_updateStatusButton_clicked();

private:
    Ui::menu_restaurant *ui;
    QString selectedFoodType;
    QString selectedFoodName;
    int selectedItemIndex;
    QString currentRestaurantUsername;
    int currentRestaurantId;
};

#endif // MENU_RESTAURANT_H
