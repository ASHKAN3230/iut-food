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
    , menuExists(false)
    , infoExists(false)
    , currentRestaurantId(-1)
{
    ui->setupUi(this);

    currentRestaurantUsername = username;

    connect(this, &restaurant_auth::click_back, this, &restaurant_auth::send_message);
    connect(this, &restaurant_auth::receive_message, this, &restaurant_auth::on_back_button_clicked);

    // Connect button signals
    connect(ui->continue_button, &QPushButton::clicked, this, &restaurant_auth::on_continue_button_clicked);
    connect(ui->setup_menu_button, &QPushButton::clicked, this, &restaurant_auth::on_setup_menu_button_clicked);
    connect(ui->back_button, &QPushButton::clicked, this, &restaurant_auth::on_back_button_clicked);
    connect(ui->save_info_button, &QPushButton::clicked, this, &restaurant_auth::on_save_info_button_clicked);

    // Connect to NetworkManager's restaurantsReceived signal
    connect(NetworkManager::getInstance(), &NetworkManager::restaurantsReceived, this, &restaurant_auth::onRestaurantsReceived);

    socket.connectToHost("127.0.0.1", 6006);
    if(socket.waitForConnected(1000))
    {
        // receive_message();
    }
    
    check_restaurant_info_status();
    check_restaurant_menu_status();
    update_status_display();
}

restaurant_auth::~restaurant_auth()
{
    delete ui;
}

void restaurant_auth::check_restaurant_info_status()
{
    // Fetch restaurant info from server instead of local DB
    NetworkManager::getInstance()->getRestaurants();
}

// Add a slot to handle the server response
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
            // Disable editing if info exists
            ui->restaurantNameEdit->setEnabled(false);
            ui->restaurantTypeCombo->setEnabled(false);
            ui->restaurantAddressEdit->setEnabled(false);
            ui->restaurantDescEdit->setEnabled(false);
            ui->save_info_button->setEnabled(false);
            break;
        }
    }
    if (!infoExists) {
        // Enable editing for new restaurants
        ui->restaurantNameEdit->setEnabled(true);
        ui->restaurantTypeCombo->setEnabled(true);
        ui->restaurantAddressEdit->setEnabled(true);
        ui->restaurantDescEdit->setEnabled(true);
        ui->save_info_button->setEnabled(true);
    }
    update_status_display();
}

void restaurant_auth::check_restaurant_menu_status()
{
    // Check if there are any menu items in the database
    if(currentRestaurantId == -1)
    {
        menuExists = false;
        return;
    }
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("iut_food.db");
    
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Could not open database: " + db.lastError().text());
        menuExists = false;
        return;
    }
    
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM menu_items WHERE restaurant_id = ?");
    query.addBindValue(currentRestaurantId);
    
    if (query.exec() && query.next()) {
        int count = query.value(0).toInt();
        menuExists = (count > 0);
    } else {
        menuExists = false;
    }
    
    db.close();
}

bool restaurant_auth::has_menu_items()
{
    return menuExists;
}

bool restaurant_auth::has_restaurant_info()
{
    return infoExists;
}

void restaurant_auth::save_restaurant_info()
{
    QString name = ui->restaurantNameEdit->text().trimmed();
    QString type = ui->restaurantTypeCombo->currentText();
    QString address = ui->restaurantAddressEdit->text().trimmed();
    QString description = ui->restaurantDescEdit->toPlainText().trimmed();
    
    if(name.isEmpty() || address.isEmpty())
    {
        QMessageBox::warning(this, "Input Error", "Please fill in restaurant name and address!");
        return;
    }

    // Prepare data for server
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

    // Send to server using NetworkManager
    NetworkManager::getInstance()->createRestaurant(data);
    // UI will update in onRestaurantCreated
}

void restaurant_auth::update_status_display()
{
    QString statusText;
    QString statusStyle;
    
    if(!infoExists)
    {
        statusText = "⚠ Please complete your restaurant information first.";
        statusStyle = "color: rgb(255, 165, 0); font: 700 12pt 'Segoe UI'; background-color: rgba(255,165,0,30); border-radius: 5px; padding: 10px;";
        ui->continue_button->setEnabled(false);
        ui->setup_menu_button->setEnabled(false);
    }
    else if(!menuExists)
    {
        statusText = "✓ Restaurant info complete. ⚠ Menu not set up yet.";
        statusStyle = "color: rgb(255, 165, 0); font: 700 12pt 'Segoe UI'; background-color: rgba(255,165,0,30); border-radius: 5px; padding: 10px;";
        ui->continue_button->setEnabled(false);
        ui->setup_menu_button->setEnabled(true);
    }
    else
    {
        statusText = "✓ Restaurant info and menu are complete and ready!";
        statusStyle = "color: rgb(0, 128, 0); font: 700 12pt 'Segoe UI'; background-color: rgba(0,128,0,30); border-radius: 5px; padding: 10px;";
        ui->continue_button->setEnabled(true);
        ui->setup_menu_button->setEnabled(false);
        ui->setup_menu_button->setText("Menu Already Setup");
    }
    
    ui->statusLabel->setText(statusText);
    ui->statusLabel->setStyleSheet(statusStyle);
}

void restaurant_auth::create_initial_menu()
{
    if(currentRestaurantId == -1)
    {
        QMessageBox::warning(this, "Error", "Please save restaurant information first!");
        return;
    }
    
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("iut_food.db");
    
    if (!db.open()) {
        QMessageBox::critical(this, "Database Error", "Could not open database: " + db.lastError().text());
        return;
    }
    
    // Create sample menu items
    QSqlQuery query;
    query.prepare("INSERT INTO menu_items (restaurant_id, food_type, food_name, food_details, price) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(currentRestaurantId);
    query.addBindValue("Main Course");
    query.addBindValue("Sample Dish");
    query.addBindValue("Delicious sample dish with fresh ingredients");
    query.addBindValue(25000);
    
    if (query.exec()) {
        QMessageBox::information(this, "Menu Created", "Initial menu structure has been created in the database. You can now add your restaurant's menu items.");
        menuExists = true;
        update_status_display();
    } else {
        QMessageBox::critical(this, "Error", "Could not create menu item: " + query.lastError().text());
    }
    
    db.close();
}

void restaurant_auth::on_back_button_clicked()
{
    menu_restaurant *mr = new menu_restaurant(currentRestaurantUsername);
    mr->setAttribute(Qt::WA_DeleteOnClose);
    mr->showMaximized();
    this->close();
}

void restaurant_auth::on_continue_button_clicked()
{
    if(!infoExists)
    {
        QMessageBox::warning(this, "Incomplete Setup", "Please complete your restaurant information first!");
        return;
    }
    
    if(!menuExists)
    {
        QMessageBox::warning(this, "Incomplete Setup", "Please set up your menu first!");
        return;
    }
    
    // Restaurant has complete info and menu, proceed to menu management
    menu_restaurant *mr = new menu_restaurant(currentRestaurantUsername);
    mr->setAttribute(Qt::WA_DeleteOnClose);
    mr->showMaximized();
    this->close();
}

void restaurant_auth::on_setup_menu_button_clicked()
{
    if(!infoExists)
    {
        QMessageBox::warning(this, "Incomplete Setup", "Please complete your restaurant information first!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Setup Menu", 
        "Would you like to create an initial menu structure?\n\nThis will create a sample menu item in the database that you can then edit or replace with your own menu items.",
        QMessageBox::Yes | QMessageBox::No);
    
    if(reply == QMessageBox::Yes)
    {
        // Create initial menu structure
        create_initial_menu();
        
        // Show menu management interface
        menu_restaurant *mr = new menu_restaurant(currentRestaurantUsername);
        mr->setAttribute(Qt::WA_DeleteOnClose);
        mr->showMaximized();
        this->close();
    }
}

void restaurant_auth::on_save_info_button_clicked()
{
    save_restaurant_info();
}

void restaurant_auth::send_message()
{
    if(socket.state() == QTcpSocket::ConnectedState)
    {
        QByteArray byte(this->message.c_str(), this->message.length());
        if(socket.write(byte) > 0)
        {
            if(!socket.waitForBytesWritten(1000))
            {
                QMessageBox::information(this, "not success", "message not send!!!");
            }
        }
    }
}

void restaurant_auth::onRestaurantCreated(bool success)
{
    if (success) {
        QMessageBox::information(this, "Success", "Restaurant information saved on server!");
        infoExists = true;
        check_restaurant_info_status();
        update_status_display();
        ui->restaurantNameEdit->setEnabled(false);
        ui->restaurantTypeCombo->setEnabled(false);
        ui->restaurantAddressEdit->setEnabled(false);
        ui->restaurantDescEdit->setEnabled(false);
        ui->save_info_button->setEnabled(false);
    } else {
        QMessageBox::critical(this, "Error", "Failed to save restaurant information on server.");
    }
}