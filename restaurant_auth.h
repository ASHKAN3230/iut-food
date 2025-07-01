#ifndef RESTAURANT_AUTH_H
#define RESTAURANT_AUTH_H

#include <QWidget>
#include <QString>
#include <QMap>
#include <QPair>
#include <string>
#include <QTcpSocket>

namespace Ui {
class restaurant_auth;
}

class restaurant_auth : public QWidget
{
    Q_OBJECT

public:
    explicit restaurant_auth(const QString &username, QWidget *parent = nullptr);
    ~restaurant_auth();

    void check_restaurant_info_status();
    void save_restaurant_info();
    void update_status_display();

signals:
    void receive_message();

public slots:
    void on_save_info_button_clicked();
    void onRestaurantCreated(bool success);
    void onRestaurantsReceived(const QJsonArray &restaurants);

private:
    Ui::restaurant_auth *ui;
    std::string message;
    QString currentRestaurantUsername;
    bool infoExists;
    int currentRestaurantId;
};

#endif // RESTAURANT_AUTH_H