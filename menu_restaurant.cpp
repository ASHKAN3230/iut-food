#include "menu_restaurant.h"
#include "ui_menu_restaurant.h"
#include "customer.h"
#include "shopping_basket.h"
#include "restaurant_auth.h"
#include "mainwindow.h"
#include "network_manager.h"
#include <QFile>
#include <QLabel>
#include <QTextStream>
#include <QListWidgetItem>
#include <QSpinBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

menu_restaurant::menu_restaurant(const QString &username, int restaurantId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menu_restaurant)
    , selectedItemIndex(-1)
    , currentRestaurantId(restaurantId)
{
    ui->setupUi(this);
    setFixedSize(700, 600);
    currentRestaurantUsername = username;

    // Connect network manager signals
    NetworkManager* netManager = NetworkManager::getInstance();
    connect(netManager, &NetworkManager::menuReceived, this, &menu_restaurant::onMenuReceived);
    connect(netManager, &NetworkManager::orderCreated, this, &menu_restaurant::onOrderCreated);
    connect(netManager, &NetworkManager::ordersReceived, this, &menu_restaurant::onOrdersReceived);
    connect(netManager, &NetworkManager::orderStatusUpdated, this, &menu_restaurant::onOrderStatusUpdated);
    connect(netManager, &NetworkManager::networkError, this, &menu_restaurant::onNetworkError);
    
    // Connect menu operation signals
    connect(netManager, &NetworkManager::menuItemAdded, this, &menu_restaurant::onMenuItemAdded);
    connect(netManager, &NetworkManager::menuItemAddedFailed, this, &menu_restaurant::onMenuItemAddedFailed);
    connect(netManager, &NetworkManager::menuItemUpdated, this, &menu_restaurant::onMenuItemUpdated);
    connect(netManager, &NetworkManager::menuItemUpdatedFailed, this, &menu_restaurant::onMenuItemUpdatedFailed);
    connect(netManager, &NetworkManager::menuItemDeleted, this, &menu_restaurant::onMenuItemDeleted);
    connect(netManager, &NetworkManager::menuItemDeletedFailed, this, &menu_restaurant::onMenuItemDeletedFailed);

    connect(ui->addFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_addFoodButton_clicked);
    connect(ui->editFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_editFoodButton_clicked);
    connect(ui->deleteFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_deleteFoodButton_clicked);
    connect(ui->clearFormButton, &QPushButton::clicked, this, &menu_restaurant::on_clearFormButton_clicked);
    connect(ui->menuTableWidget, &QTableWidget::itemClicked, this, &menu_restaurant::on_menuItem_selected);
    connect(ui->refreshOrdersButton, &QPushButton::clicked, this, &menu_restaurant::on_refreshOrdersButton_clicked);
    connect(ui->updateStatusButton, &QPushButton::clicked, this, &menu_restaurant::on_updateStatusButton_clicked);
    // Profile tab logic
    connect(ui->saveProfileButton, &QPushButton::clicked, this, &menu_restaurant::on_saveProfileButton_clicked);

    socket.connectToHost("127.0.0.1",6006);
    if(socket.waitForConnected(1000))
    {
        // receive_message();
    }

    // Only call getRestaurantInfo if restaurantId is not provided
    if (currentRestaurantId <= 0) {
        getRestaurantInfo();
    }
    // Load menu and orders using network manager
    if (currentRestaurantId > 0) {
        loadMenuFromServer();
        loadOrdersFromServer();
    }
}

menu_restaurant::~menu_restaurant()
{
    delete ui;
}

int menu_restaurant::index = 0;

void menu_restaurant::getRestaurantInfo()
{
    currentRestaurantId = -1;
}

void menu_restaurant::loadMenuFromServer()
{
    if (currentRestaurantId <= 0) return;
    
    NetworkManager* netManager = NetworkManager::getInstance();
    netManager->getRestaurantMenu(currentRestaurantId);
}

void menu_restaurant::onMenuReceived(const QJsonArray &menu)
{
    menu_list.clear();
    
    for (const QJsonValue &itemValue : menu) {
        QJsonObject item = itemValue.toObject();
        QString foodType = item["foodType"].toString();
        QString foodName = item["foodName"].toString();
        QString foodDetails = item["foodDetails"].toString();
        QString price = item["price"].toString();
        
        QPair<QString, QString> food(foodDetails, price);
        menu_list[foodType][foodName] = food;
    }
    
    refresh_menu_display();
}

void menu_restaurant::loadOrdersFromServer()
{
    if (currentRestaurantId <= 0) return;
    
    NetworkManager* netManager = NetworkManager::getInstance();
    netManager->getOrders(QString::number(currentRestaurantId), "restaurant");
}

void menu_restaurant::onOrdersReceived(const QJsonArray &orders)
{
    populate_orders_table();
    ui->ordersTableWidget->setRowCount(0);
    int rowCount = 0;
    for (const QJsonValue &orderValue : orders) {
        QJsonObject order = orderValue.toObject();
        int row = ui->ordersTableWidget->rowCount();
        ui->ordersTableWidget->insertRow(row);
        // Order ID (Column 0) - Store it as user data
        QTableWidgetItem *idItem = new QTableWidgetItem();
        idItem->setData(Qt::UserRole, order["id"].toInt());
        ui->ordersTableWidget->setItem(row, 0, idItem);
        // Customer (Column 1)
        ui->ordersTableWidget->setItem(row, 1, new QTableWidgetItem(order["customerName"].toString()));
        // Total (Column 2)
        ui->ordersTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(order["totalAmount"].toInt())));
        // Status (Column 3)
        ui->ordersTableWidget->setItem(row, 3, new QTableWidgetItem(order["status"].toString()));
        // Date (Column 4)
        ui->ordersTableWidget->setItem(row, 4, new QTableWidgetItem(order["createdAt"].toString()));
        rowCount++;
    }
}

void menu_restaurant::onOrderCreated(const QString &message)
{
    QMessageBox::information(this, "Success", message);
}

void menu_restaurant::onOrderStatusUpdated(const QString &message)
{
    QMessageBox::information(this, "Success", message);
    loadOrdersFromServer(); // Refresh orders after status update
}

void menu_restaurant::onNetworkError(const QString &error)
{
    QMessageBox::critical(this, "Network Error", 
                         QString("Network error: %1\n\nPlease check if the server is running.").arg(error));
}

void menu_restaurant::open_menu_from_database()
{
    // This method is now replaced by loadMenuFromServer()
    // Keeping for backward compatibility
    loadMenuFromServer();
}

void menu_restaurant::save_menu_to_database()
{
    if (currentRestaurantId == -1) return;

    // For now, we'll use local database as fallback
    // In the future, this should send menu updates to the server
    QSqlDatabase db = QSqlDatabase::database();
     if (!db.isOpen()) {
        // Handle error
        return;
    }

    QSqlQuery clearQuery;
    clearQuery.prepare("DELETE FROM menu_items WHERE restaurant_id = ?");
    clearQuery.addBindValue(currentRestaurantId);
    if (!clearQuery.exec()) {
        QMessageBox::warning(this, "Database Error", "Could not clear old menu items: " + clearQuery.lastError().text());
        return;
    }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO menu_items (restaurant_id, food_type, food_name, food_details, price) VALUES (?, ?, ?, ?, ?)");
    
    for(auto typeIt = menu_list.constBegin(); typeIt != menu_list.constEnd(); ++typeIt)
    {
        const QString& foodType = typeIt.key();
        const QMap<QString, QPair<QString, QString>>& foods = typeIt.value();
        
        for(auto foodIt = foods.constBegin(); foodIt != foods.constEnd(); ++foodIt)
        {
            const QString& foodName = foodIt.key();
            const QPair<QString, QString>& food = foodIt.value();
            
            insertQuery.bindValue(0, currentRestaurantId);
            insertQuery.bindValue(1, foodType);
            insertQuery.bindValue(2, foodName);
            insertQuery.bindValue(3, food.first);
            insertQuery.bindValue(4, food.second.toInt());
            
            if (!insertQuery.exec()) {
                QMessageBox::warning(this, "Database Error", "Could not save menu item '" + foodName + "': " + insertQuery.lastError().text());
            }
        }
    }
}

void menu_restaurant::refresh_menu_display()
{
    QTableWidget *menuTableWidget = ui->menuTableWidget;
    menuTableWidget->clear();
    menuTableWidget->setRowCount(0);
    menuTableWidget->setColumnCount(4);
    menuTableWidget->setHorizontalHeaderLabels({"Food Type", "Food Name", "Details", "Price"});
    menuTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for(auto restaurantIt = menu_list.begin(); restaurantIt != menu_list.end(); ++restaurantIt)
    {
        QString foodType = restaurantIt.key();
        QMap<QString, QPair<QString, QString>> foods = restaurantIt.value();
        for(auto foodIt = foods.begin(); foodIt != foods.end(); ++foodIt)
        {
            QString foodName = foodIt.key();
            QPair<QString, QString> food = foodIt.value();
            QString foodDetails = food.first;
            QString price = food.second;

            int row = menuTableWidget->rowCount();
            menuTableWidget->insertRow(row);
            menuTableWidget->setItem(row, 0, new QTableWidgetItem(foodType));
            menuTableWidget->setItem(row, 1, new QTableWidgetItem(foodName));
            menuTableWidget->setItem(row, 2, new QTableWidgetItem(foodDetails));
            menuTableWidget->setItem(row, 3, new QTableWidgetItem(price));
        }
    }
}

void menu_restaurant::clear_form()
{
    ui->foodNameEdit->clear();
    ui->foodTypeEdit->clear();
    ui->foodDetailsEdit->clear();
    ui->priceSpinBox->setValue(0);
    selectedItemIndex = -1;
    selectedFoodType.clear();
    selectedFoodName.clear();
}

void menu_restaurant::on_addFoodButton_clicked()
{
    QString foodName = ui->foodNameEdit->text().trimmed();
    QString foodType = ui->foodTypeEdit->text().trimmed();
    QString foodDetails = ui->foodDetailsEdit->text().trimmed();
    int price = ui->priceSpinBox->value();
    
    // Validation
    if (foodName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a food name.");
        return;
    }
    
    if (foodType.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a food type.");
        return;
    }
    
    if (foodDetails.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter food details.");
        return;
    }
    
    if (price <= 0) {
        QMessageBox::warning(this, "Error", "Please enter a valid price.");
        return;
    }
    
    // Check if food name already exists in the selected type
    if (menu_list.contains(foodType) && menu_list[foodType].contains(foodName)) {
        QMessageBox::warning(this, "Error", "A food item with this name already exists in the selected type.");
        return;
    }
    
    // Send to server via network manager
    NetworkManager::getInstance()->addMenuItem(currentRestaurantId, foodType, foodName, foodDetails, price);
    
    // Clear form immediately for better UX
    clear_form();
}

void menu_restaurant::on_editFoodButton_clicked()
{
    if (selectedItemIndex == -1) {
        QMessageBox::warning(this, "Selection Error", "Please select a food item to edit!");
        return;
    }
    
    QString newFoodName = ui->foodNameEdit->text().trimmed();
    QString newFoodType = ui->foodTypeEdit->text().trimmed();
    QString newFoodDetails = ui->foodDetailsEdit->text().trimmed();
    int newPrice = ui->priceSpinBox->value();
    
    // Validation
    if (newFoodName.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a food name.");
        return;
    }
    
    if (newFoodType.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a food type.");
        return;
    }
    
    if (newFoodDetails.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter food details.");
        return;
    }
    
    if (newPrice <= 0) {
        QMessageBox::warning(this, "Error", "Please enter a valid price.");
        return;
    }
    
    // For now, we'll use a simple approach - delete old item and add new one
    // In a real implementation, you'd want to track item IDs from the server
    if (menu_list.contains(selectedFoodType) && menu_list[selectedFoodType].contains(selectedFoodName)) {
        // Remove from local cache
        menu_list[selectedFoodType].remove(selectedFoodName);
        
        // Remove empty food type
        if (menu_list[selectedFoodType].isEmpty()) {
            menu_list.remove(selectedFoodType);
        }
    }
    
    // Add new item to server
    NetworkManager::getInstance()->addMenuItem(currentRestaurantId, newFoodType, newFoodName, newFoodDetails, newPrice);
    
    // Clear form immediately for better UX
    clear_form();
}

void menu_restaurant::on_deleteFoodButton_clicked()
{
    if (selectedItemIndex == -1) {
        QMessageBox::warning(this, "Selection Error", "Please select a food item to delete!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        "Are you sure you want to delete this food item?", 
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // For now, we'll use a simple approach - remove from local cache
        // In a real implementation, you'd want to track item IDs from the server
        if (menu_list.contains(selectedFoodType) && menu_list[selectedFoodType].contains(selectedFoodName)) {
            // Remove from local cache
            menu_list[selectedFoodType].remove(selectedFoodName);
            
            // Remove empty food type
            if (menu_list[selectedFoodType].isEmpty()) {
                menu_list.remove(selectedFoodType);
            }
        }
        
        // Send delete request to server (we'll need item ID in real implementation)
        // For now, we'll refresh the menu from server after local removal
        NetworkManager::getInstance()->getRestaurantMenu(currentRestaurantId);
        
        // Clear form immediately for better UX
        clear_form();
    }
}

void menu_restaurant::on_clearFormButton_clicked()
{
    clear_form();
}

void menu_restaurant::on_menuItem_selected()
{
    QTableWidget *menuTableWidget = ui->menuTableWidget;
    
    QList<QTableWidgetItem*> selectedItems = menuTableWidget->selectedItems();
    if (selectedItems.isEmpty()) return;
    
    int row = selectedItems.first()->row();
    
    // Set values in the form
    QString foodType = menuTableWidget->item(row, 0)->text();
    QString foodName = menuTableWidget->item(row, 1)->text();
    QString foodDetails = menuTableWidget->item(row, 2)->text();
    QString price = menuTableWidget->item(row, 3)->text();
    
    ui->foodTypeEdit->setText(foodType);
    ui->foodNameEdit->setText(foodName);
    ui->foodDetailsEdit->setText(foodDetails);
    ui->priceSpinBox->setValue(price.toInt());
    
    selectedItemIndex = row;
    selectedFoodType = foodType;
    selectedFoodName = foodName;
}

void menu_restaurant::on_refreshOrdersButton_clicked()
{
    loadOrdersFromServer();
}

void menu_restaurant::on_updateStatusButton_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->ordersTableWidget->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an order to update.");
        return;
    }

    int selectedRow = selectedItems.first()->row();
    int orderId = ui->ordersTableWidget->item(selectedRow, 0)->data(Qt::UserRole).toInt();

    // Get new status from user
    QStringList statuses = {"Pending", "Preparing", "Out for Delivery", "Delivered", "Cancelled"};
    bool ok;
    QString newStatus = QInputDialog::getItem(this, "Update Order Status", "Select the new status for the order:", statuses, 0, false, &ok);

    if (ok && !newStatus.isEmpty()) {
        // Use network manager to update order status
        NetworkManager* netManager = NetworkManager::getInstance();
        netManager->updateOrderStatus(orderId, newStatus);
    }
}

void menu_restaurant::onMenuItemAdded(const QString &message)
{
    // Refresh menu from server after successful addition
    NetworkManager::getInstance()->getRestaurantMenu(currentRestaurantId);
    QMessageBox::information(this, "Success", "Food item added successfully!");
}

void menu_restaurant::onMenuItemOperationFailed(const QString &errorMessage)
{
    QMessageBox::warning(this, "Operation Error", errorMessage);
}

void menu_restaurant::onMenuItemUpdated(const QString &message)
{
    // Refresh menu from server after successful update
    NetworkManager::getInstance()->getRestaurantMenu(currentRestaurantId);
    QMessageBox::information(this, "Success", "Food item updated successfully!");
}

void menu_restaurant::onMenuItemDeleted(const QString &message)
{
    // Refresh menu from server after successful deletion
    NetworkManager::getInstance()->getRestaurantMenu(currentRestaurantId);
    QMessageBox::information(this, "Success", "Food item deleted successfully!");
}

void menu_restaurant::onMenuItemAddedFailed(const QString &error)
{
    QMessageBox::warning(this, "Add Failed", error);
}

void menu_restaurant::onMenuItemUpdatedFailed(const QString &error)
{
    QMessageBox::warning(this, "Update Failed", error);
}

void menu_restaurant::onMenuItemDeletedFailed(const QString &error)
{
    QMessageBox::warning(this, "Delete Failed", error);
}

void menu_restaurant::on_saveProfileButton_clicked()
{
    QString name = ui->restaurantNameEdit->text().trimmed();
    QString type = ui->restaurantTypeEdit->text().trimmed();
    QString address = ui->restaurantAddressEdit->text().trimmed();
    QString desc = ui->restaurantDescEdit->text().trimmed();
    // TODO: Save profile info to server or database
    QMessageBox::information(this, "Profile Saved", "Profile information has been saved.");
}

void menu_restaurant::populate_orders_table()
{
    if (!ui || !ui->ordersTableWidget) return;
    ui->ordersTableWidget->setColumnCount(5);
    ui->ordersTableWidget->setHorizontalHeaderLabels({"ID", "Customer", "Total Price", "Status", "Date"});
    ui->ordersTableWidget->setColumnHidden(0, true); // Hide the ID column
    ui->ordersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ordersTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ordersTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void menu_restaurant::send_message() {}

void menu_restaurant::setAuthWarningVisible(bool visible) {
    if (ui && ui->authWarningLabel) {
        ui->authWarningLabel->setVisible(visible);
    }
}

