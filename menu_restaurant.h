#ifndef MENU_RESTAURANT_H
#define MENU_RESTAURANT_H

#include <QWidget>
#include <QString>
#include <QPair>
#include <QJsonArray>
#include <string>
#include <QTcpSocket>
#include <QTcpServer>

namespace Ui {
class menu_restaurant;
}

class menu_restaurant : public QWidget
{
    Q_OBJECT

public:
    explicit menu_restaurant(const QString &username, int restaurantId = -1, QWidget *parent = nullptr);
    ~menu_restaurant();

    void click_back_button();

    void open_menu_from_database();

    void receive_message();

    void save_menu_to_database();

    void refresh_menu_display();

    void clear_form();

    void load_orders();

    void populate_orders_table();

    void setAuthWarningVisible(bool visible);

    static int index;

    std::string message;

    QTcpSocket socket;

    QMap<QString,QMap<QString,QPair<QString,QString>>> menu_list;

signals:

    void click_server();

public slots:
    void send_message();

    void on_addFoodButton_clicked();

    void on_editFoodButton_clicked();

    void on_deleteFoodButton_clicked();

    void on_clearFormButton_clicked();

    void on_menuItem_selected();

    void on_refreshOrdersButton_clicked();

    void on_updateStatusButton_clicked();
    
    // Network manager slots
    void onMenuReceived(const QJsonArray &menu);
    void onOrdersReceived(const QJsonArray &orders);
    void onOrderCreated(const QString &message);
    void onOrderStatusUpdated(const QString &message);
    void onNetworkError(const QString &error);
    
    // Menu operation slots
    void onMenuItemAdded(const QString &message);
    void onMenuItemAddedFailed(const QString &error);
    void onMenuItemUpdated(const QString &message);
    void onMenuItemUpdatedFailed(const QString &error);
    void onMenuItemDeleted(const QString &message);
    void onMenuItemDeletedFailed(const QString &error);
    void onMenuItemOperationFailed(const QString &error);
    // Profile tab slot
    void on_saveProfileButton_clicked();

private:
    Ui::menu_restaurant *ui;
    QString selectedFoodType;
    QString selectedFoodName;
    int selectedItemIndex;
    QString currentRestaurantUsername;
    int currentRestaurantId;
    
    // Network manager methods
    void getRestaurantInfo();
    void loadMenuFromServer();
    void loadOrdersFromServer();
};

#endif // MENU_RESTAURANT_H
