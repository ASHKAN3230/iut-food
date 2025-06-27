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

    void check_restaurant_menu_status();
    void check_restaurant_info_status();
    bool has_menu_items();
    bool has_restaurant_info();
    void create_initial_menu();
    void save_restaurant_info();
    void update_status_display();

signals:
    void click_back();
    void click_server();
    void receive_message();

public slots:
    void on_back_button_clicked();
    void on_continue_button_clicked();
    void on_setup_menu_button_clicked();
    void on_save_info_button_clicked();
    void send_message();

private:
    Ui::restaurant_auth *ui;
    std::string message;
    QTcpSocket socket;
    QString currentRestaurantUsername;
    QMap<QString,QMap<QString,QPair<QString,QString>>> menu_list;
    bool menuExists;
    bool infoExists;
    int currentRestaurantId;
};

#endif // RESTAURANT_AUTH_H 