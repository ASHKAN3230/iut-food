#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QWidget>
#include<QMap>
#include<QString>
#include<QVector>
#include<QPair>
#include<string>
#include<QTcpSocket>
#include<QTcpServer>
#include<QJsonArray>

namespace Ui {
class customer;
}

class customer : public QWidget
{
    Q_OBJECT

public:
    explicit customer(const QString &username, int userId, QWidget *parent = nullptr);

    void click_search_button();

    void click_back_button();

    void open_file();

    void receive_message();

    std::string message;

    QTcpSocket socket;

    QMap<QString,QMap<QString,QPair<QVector<QString>,QPair<int,int>>>> restaurant_list;

    ~customer();

    // New API-driven UI methods
    void fetchAndDisplayRestaurants();
    void fetchAndDisplayMenu(int restaurantId, const QString &restaurantName);
    void displayRestaurants(const QJsonArray &restaurants);
    void displayMenu(const QJsonArray &menu, const QString &restaurantName, int restaurantId);
    void orderFood(int restaurantId, int foodId, const QString &restaurantName, const QString &foodName, int price);
    void fetchAndDisplayOrders();
    void displayOrders(const QJsonArray &orders);

signals:

    void click_search();

    void click_back();

    void click_server();

public slots:

    void on_search_button_clicked();

    void on_back_button_clicked();

    void open_next_window(int new_index);

    void send_message();

private:
    Ui::customer *ui;
    QString currentUsername;
    int currentUserId = -1;
};

#endif // CUSTOMER_H
