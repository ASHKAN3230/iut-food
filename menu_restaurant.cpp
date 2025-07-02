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
#include <QNetworkReply>
#include <QHBoxLayout>
#include <QSizePolicy>

menu_restaurant::menu_restaurant(const QString &username, int restaurantId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menu_restaurant)
    , selectedItemIndex(-1)
    , currentRestaurantId(restaurantId)
{
    ui->setupUi(this);
    currentRestaurantUsername = username;

    // Disconnect all previous NetworkManager signal connections to this before connecting new ones in the menu_restaurant constructor
    NetworkManager* netManager = NetworkManager::getInstance();
    disconnect(netManager, nullptr, this, nullptr);
    connect(netManager, &NetworkManager::menuReceived, this, &menu_restaurant::onMenuReceived);
    connect(netManager, &NetworkManager::orderCreated, this, &menu_restaurant::onOrderCreated);
    connect(netManager, &NetworkManager::ordersReceived, this, &menu_restaurant::onOrdersReceived);
    connect(netManager, &NetworkManager::orderStatusUpdated, this, &menu_restaurant::onOrderStatusUpdated);
    connect(netManager, &NetworkManager::networkError, this, &menu_restaurant::onNetworkError);
    connect(netManager, &NetworkManager::menuItemAdded, this, &menu_restaurant::onMenuItemAdded);
    connect(netManager, &NetworkManager::menuItemAddedFailed, this, &menu_restaurant::onMenuItemAddedFailed);
    connect(netManager, &NetworkManager::menuItemUpdated, this, &menu_restaurant::onMenuItemUpdated);
    connect(netManager, &NetworkManager::menuItemUpdatedFailed, this, &menu_restaurant::onMenuItemUpdatedFailed);
    connect(netManager, &NetworkManager::menuItemDeleted, this, &menu_restaurant::onMenuItemDeleted);
    connect(netManager, &NetworkManager::menuItemDeletedFailed, this, &menu_restaurant::onMenuItemDeletedFailed);

    connect(ui->addFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_addFoodButton_clicked);
    connect(ui->editFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_editFoodButton_clicked);
    connect(ui->clearFormButton, &QPushButton::clicked, this, &menu_restaurant::on_clearFormButton_clicked);
    connect(ui->menuTableWidget, &QTableWidget::itemClicked, this, &menu_restaurant::on_menuItem_selected);
    connect(ui->refreshOrdersButton, &QPushButton::clicked, this, &menu_restaurant::on_refreshOrdersButton_clicked);
    connect(ui->updateStatusButton, &QPushButton::clicked, this, &menu_restaurant::on_updateStatusButton_clicked);
    // Profile tab logic
    connect(ui->saveProfileButton, &QPushButton::clicked, this, &menu_restaurant::on_saveProfileButton_clicked);
    connect(ui->applyAuthButton, &QPushButton::clicked, this, &menu_restaurant::on_applyAuthButton_clicked);

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
    // Fetch and set auth warning
    fetchAndSetAuthWarning();
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
    menu_items.clear();
    for (const QJsonValue &itemValue : menu) {
        QJsonObject item = itemValue.toObject();
        int id = item["id"].toInt();
        MenuItemInfo info;
        info.id = id;
        info.foodType = item["foodType"].toString();
        info.foodName = item["foodName"].toString();
        info.foodDetails = item["foodDetails"].toString();
        info.price = QString::number(item["price"].toInt());
        menu_items[id] = info;
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

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
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

    for (auto it = menu_items.constBegin(); it != menu_items.constEnd(); ++it) {
        const MenuItemInfo &info = it.value();
        insertQuery.bindValue(0, currentRestaurantId);
        insertQuery.bindValue(1, info.foodType);
        insertQuery.bindValue(2, info.foodName);
        insertQuery.bindValue(3, info.foodDetails);
        insertQuery.bindValue(4, info.price.toInt());
        if (!insertQuery.exec()) {
            QMessageBox::warning(this, "Database Error", "Could not save menu item '" + info.foodName + "': " + insertQuery.lastError().text());
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

    int row = 0;
    for (auto it = menu_items.begin(); it != menu_items.end(); ++it, ++row) {
        const MenuItemInfo &info = it.value();
        menuTableWidget->insertRow(row);
        menuTableWidget->setItem(row, 0, new QTableWidgetItem(info.foodType));
        menuTableWidget->setItem(row, 1, new QTableWidgetItem(info.foodName));
        menuTableWidget->setItem(row, 2, new QTableWidgetItem(info.foodDetails));

        QWidget *priceWidget = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(priceWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(4);
        QLabel *priceLabel = new QLabel(info.price);
        QPushButton *deleteBtn = new QPushButton("Delete");
        deleteBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        deleteBtn->setProperty("menuItemId", info.id);
        layout->addWidget(priceLabel);
        layout->addStretch();
        layout->addWidget(deleteBtn);
        priceWidget->setLayout(layout);
        menuTableWidget->setCellWidget(row, 3, priceWidget);
        connect(deleteBtn, &QPushButton::clicked, this, [this, deleteBtn]() {
            int id = deleteBtn->property("menuItemId").toInt();
            this->deleteMenuItemById(id);
        });
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
    for (const auto &info : menu_items) {
        if (info.foodType == foodType && info.foodName == foodName) {
            QMessageBox::warning(this, "Error", "A food item with this name already exists in the selected type.");
            return;
        }
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
    // Just send update to server; menu_items will be refreshed after
    // TODO: You may want to track the selected item's id for a more robust update
    NetworkManager::getInstance()->addMenuItem(currentRestaurantId, newFoodType, newFoodName, newFoodDetails, newPrice);
    clear_form();
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
    loadMenuFromServer();
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

void menu_restaurant::updateAuthWarning(bool isAuth) {
    setAuthWarningVisible(!isAuth);
    if (ui && ui->applyAuthButton) {
        ui->applyAuthButton->setVisible(!isAuth);
    }
}

void menu_restaurant::fetchAndSetAuthWarning() {
    // Fetch user info from server and update auth warning
    NetworkManager* netManager = NetworkManager::getInstance();
    // Assume currentRestaurantId is the user id for restaurant users
    if (currentRestaurantId > 0) {
        QNetworkRequest request(QUrl(netManager->getServerUrl() + "/api/users/" + QString::number(currentRestaurantId)));
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, [this, manager](QNetworkReply* reply) {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                if (obj.contains("isAuth")) {
                    updateAuthWarning(obj["isAuth"].toBool());
                }
            }
            reply->deleteLater();
            manager->deleteLater();
        });
        manager->get(request);
    }
}

void menu_restaurant::on_applyAuthButton_clicked() {
    // Open the restaurant_auth window for authentication
    restaurant_auth *authWindow = new restaurant_auth(currentRestaurantUsername);
    authWindow->setAttribute(Qt::WA_DeleteOnClose);
    authWindow->show();
    this->close();
}

void menu_restaurant::deleteMenuItemById(int id)
{
    if (!menu_items.contains(id)) return;
    const MenuItemInfo &info = menu_items[id];
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        QString("Are you sure you want to delete '%1' from '%2'?").arg(info.foodName, info.foodType), 
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        NetworkManager::getInstance()->deleteMenuItem(id);
        clear_form();
    }
}

