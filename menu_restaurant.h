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

struct MenuItemInfo {
    int id;
    QString foodType;
    QString foodName;
    QString foodDetails;
    QString price;
};

class menu_restaurant : public QWidget
{
    Q_OBJECT

public:
    explicit menu_restaurant(const QString &username, int restaurantId = -1, int userId = -1, QWidget *parent = nullptr);
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

    QMap<int, MenuItemInfo> menu_items; // id -> info

signals:

    void click_server();

public slots:
    void send_message();

    void on_addFoodButton_clicked();

    void on_editFoodButton_clicked();

    void on_clearFormButton_clicked();

    void on_menuItem_selected();

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
    void updateAuthWarning(bool isAuth);
    void on_applyAuthButton_clicked();

protected:
    void changeEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    Ui::menu_restaurant *ui;
    QString selectedFoodType;
    QString selectedFoodName;
    int selectedItemIndex;
    QString currentRestaurantUsername;
    int currentRestaurantId;
    int currentUserId;
    
    // Network manager methods
    void getRestaurantInfo();
    void loadMenuFromServer();
    void loadOrdersFromServer();
    void fetchAndSetAuthWarning();

    void deleteMenuItem(const QString &foodType, const QString &foodName);
    void deleteMenuItemById(int id);

    void checkPendingApplication(bool isAuth);
};

#endif // MENU_RESTAURANT_H
