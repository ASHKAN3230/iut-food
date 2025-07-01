#include "restaurant_auth.h"
#include "ui_restaurant_auth.h"
#include "mainwindow.h"
#include "menu_restaurant.h"
#include "network_manager.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QApplication>

restaurant_auth::restaurant_auth(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::restaurant_auth)
    , infoExists(false)
    , currentRestaurantId(-1)
{
    ui->setupUi(this);
    currentRestaurantUsername = username;
    connect(ui->save_info_button, &QPushButton::clicked, this, &restaurant_auth::on_save_info_button_clicked);
    connect(NetworkManager::getInstance(), &NetworkManager::restaurantsReceived, this, &restaurant_auth::onRestaurantsReceived);
    check_restaurant_info_status();
    update_status_display();
}

restaurant_auth::~restaurant_auth()
{
    delete ui;
}

void restaurant_auth::check_restaurant_info_status()
{
    NetworkManager::getInstance()->getRestaurants();
}

void restaurant_auth::onRestaurantsReceived(const QJsonArray &restaurants)
{
    infoExists = false;
    currentRestaurantId = -1;
    for (const QJsonValue &val : restaurants) {
        QJsonObject obj = val.toObject();
        if (obj["username"].toString() == currentRestaurantUsername) {
            infoExists = true;
            currentRestaurantId = obj["id"].toInt();
            ui->restaurantNameEdit->setText(obj["name"].toString());
            ui->restaurantTypeCombo->setCurrentText(obj["type"].toString());
            ui->restaurantAddressEdit->setText(obj["location"].toString());
            ui->restaurantDescEdit->setPlainText(obj["description"].toString());
            break;
        }
    }
    update_status_display();
}

void restaurant_auth::save_restaurant_info()
{
    QString name = ui->restaurantNameEdit->text().trimmed();
    QString type = ui->restaurantTypeCombo->currentText();
    QString address = ui->restaurantAddressEdit->text().trimmed();
    QString description = ui->restaurantDescEdit->toPlainText().trimmed();
    if(name.isEmpty() || address.isEmpty()) {
        ui->statusLabel->setText("Please fill in restaurant name and address!");
        ui->statusLabel->setStyleSheet("color: #b71c1c; font: 10pt 'Segoe UI';");
        return;
    }
    int minPrice = 10000;
    int maxPrice = 100000;
    QJsonObject data;
    data["name"] = name;
    data["type"] = type;
    data["location"] = address;
    data["description"] = description;
    data["minPrice"] = minPrice;
    data["maxPrice"] = maxPrice;
    data["username"] = currentRestaurantUsername;
    NetworkManager::getInstance()->createRestaurant(data);
}

void restaurant_auth::update_status_display()
{
    if(!infoExists) {
        ui->statusLabel->setText("Status: Please complete your restaurant information.");
    } else {
        ui->statusLabel->setText("Status: Restaurant info saved.");
    }
}

void restaurant_auth::on_save_info_button_clicked()
{
    save_restaurant_info();
}

void restaurant_auth::onRestaurantCreated(bool success)
{
    if (success) {
        ui->statusLabel->setText("Restaurant information saved!");
        infoExists = true;
        check_restaurant_info_status();
        update_status_display();
    } else {
        ui->statusLabel->setText("Failed to save restaurant information on server.");
    }
}