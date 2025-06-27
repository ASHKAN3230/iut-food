#include "menu_restaurant.h"
#include "ui_menu_restaurant.h"
#include "customer.h"
#include "shopping_basket.h"
#include "restaurant_auth.h"
#include "mainwindow.h"
#include<QFile>
#include<QLabel>
#include<QTextStream>
#include<QListWidgetItem>
#include<QSpinBox>
#include<QMessageBox>
#include<QInputDialog>
#include<QSqlDatabase>
#include<QSqlQuery>
#include<QSqlError>
#include<QTableWidgetItem>
#include<QHeaderView>
#include <QDateTime>

menu_restaurant::menu_restaurant(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menu_restaurant)
    , selectedItemIndex(-1)
    , currentRestaurantId(-1)
{
    ui->setupUi(this);
    currentRestaurantUsername = username;

    connect(this, &menu_restaurant::click_shopping_basket_button, this, &menu_restaurant::send_message);
    connect(this, &menu_restaurant::receive_message, this, &menu_restaurant::on_shopping_basket_button_clicked);
    QPushButton *addFoodButton = ui->foodManagementGroup->findChild<QPushButton*>("addFoodButton");
    QPushButton *editFoodButton = ui->foodManagementGroup->findChild<QPushButton*>("editFoodButton");
    QPushButton *deleteFoodButton = ui->foodManagementGroup->findChild<QPushButton*>("deleteFoodButton");
    QPushButton *clearFormButton = ui->foodManagementGroup->findChild<QPushButton*>("clearFormButton");
    QTableWidget *menuTableWidget = ui->menuDisplayGroup->findChild<QTableWidget*>("menuTableWidget");

    connect(addFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_addFoodButton_clicked);
    connect(editFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_editFoodButton_clicked);
    connect(deleteFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_deleteFoodButton_clicked);
    connect(clearFormButton, &QPushButton::clicked, this, &menu_restaurant::on_clearFormButton_clicked);
    connect(menuTableWidget, &QTableWidget::itemClicked, this, &menu_restaurant::on_menuItem_selected);
    connect(ui->profile_button, &QPushButton::clicked, this, &menu_restaurant::on_profile_button_clicked);
    connect(ui->logout_button, &QPushButton::clicked, this, &menu_restaurant::on_logout_button_clicked);
    QPushButton *refreshOrdersButton = ui->orderManagementGroup->findChild<QPushButton*>("refreshOrdersButton");
    QPushButton *updateStatusButton = ui->orderManagementGroup->findChild<QPushButton*>("updateStatusButton");
    connect(refreshOrdersButton, &QPushButton::clicked, this, &menu_restaurant::on_refreshOrdersButton_clicked);
    connect(updateStatusButton, &QPushButton::clicked, this, &menu_restaurant::on_updateStatusButton_clicked);

    socket.connectToHost("127.0.0.1",6006);
    if(socket.waitForConnected(1000))
    {
        receive_message();
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("iut_food.db");
        db.open();
    }
    QSqlQuery query;
    query.prepare("SELECT restaurant_id FROM users WHERE username = ?");
    query.addBindValue(currentRestaurantUsername);
    if (query.exec() && query.next()) {
        currentRestaurantId = query.value(0).toInt();
    } else {
        QMessageBox::critical(this, "Error", "Could not retrieve restaurant ID for user.");
    }

    open_menu_from_database();
    refresh_menu_display();
    load_orders();
}

menu_restaurant::~menu_restaurant()
{
    delete ui;
}

int menu_restaurant::index = 0;

void menu_restaurant::open_menu_from_database()
{
    if (currentRestaurantId == -1) return;
    
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        // Handle error, maybe show message
        return;
    }
    
    menu_list.clear();
    
    QSqlQuery query;
    query.prepare("SELECT food_type, food_name, food_details, price FROM menu_items WHERE restaurant_id = ? ORDER BY food_type, food_name");
    query.addBindValue(currentRestaurantId);
    
    if (query.exec()) {
        while (query.next()) {
            QString foodType = query.value(0).toString();
            QString foodName = query.value(1).toString();
            QString foodDetails = query.value(2).toString();
            QString price = query.value(3).toString();
            
            QPair<QString, QString> food(foodDetails, price);
            menu_list[foodType][foodName] = food;
        }
    } else {
        QMessageBox::warning(this, "Database Error", "Could not load menu items: " + query.lastError().text());
    }
}

void menu_restaurant::save_menu_to_database()
{
    if (currentRestaurantId == -1) return;

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
    QTableWidget *menuTableWidget = ui->menuDisplayGroup->findChild<QTableWidget*>("menuTableWidget");
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
    QLineEdit *foodTypeEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodTypeEdit");
    QLineEdit *foodNameEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodNameEdit");
    QLineEdit *foodDetailsEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodDetailsEdit");
    QSpinBox *priceSpinBox = ui->foodManagementGroup->findChild<QSpinBox*>("priceSpinBox");
    if (foodTypeEdit) foodTypeEdit->clear();
    if (foodNameEdit) foodNameEdit->clear();
    if (foodDetailsEdit) foodDetailsEdit->clear();
    if (priceSpinBox) priceSpinBox->setValue(0);
    selectedItemIndex = -1;
    selectedFoodType.clear();
    selectedFoodName.clear();
}

void menu_restaurant::on_addFoodButton_clicked()
{
    QLineEdit *foodTypeEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodTypeEdit");
    QLineEdit *foodNameEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodNameEdit");
    QLineEdit *foodDetailsEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodDetailsEdit");
    QSpinBox *priceSpinBox = ui->foodManagementGroup->findChild<QSpinBox*>("priceSpinBox");

    QString foodType = foodTypeEdit ? foodTypeEdit->text().trimmed() : "";
    QString foodName = foodNameEdit ? foodNameEdit->text().trimmed() : "";
    QString foodDetails = foodDetailsEdit ? foodDetailsEdit->text().trimmed() : "";
    QString price = priceSpinBox ? QString::number(priceSpinBox->value()) : "0";

    if(foodType.isEmpty() || foodName.isEmpty() || foodDetails.isEmpty() || price == "0")
    {
        QMessageBox::warning(this, "Input Error", "Please fill all fields with valid values!");
        return;
    }
    
    // Check if food already exists
    if(menu_list[foodType].contains(foodName))
    {
        QMessageBox::warning(this, "Duplicate Food", "A food with this name already exists in this category!");
        return;
    }
    
    QPair<QString, QString> food(foodDetails, price);
    menu_list[foodType][foodName] = food;
    
    save_menu_to_database();
    refresh_menu_display();
    clear_form();
    
    QMessageBox::information(this, "Success", "Food item added successfully!");
}

void menu_restaurant::on_editFoodButton_clicked()
{
    QLineEdit *foodTypeEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodTypeEdit");
    QLineEdit *foodNameEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodNameEdit");
    QLineEdit *foodDetailsEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodDetailsEdit");
    QSpinBox *priceSpinBox = ui->foodManagementGroup->findChild<QSpinBox*>("priceSpinBox");

    QString newFoodType = foodTypeEdit ? foodTypeEdit->text().trimmed() : "";
    QString newFoodName = foodNameEdit ? foodNameEdit->text().trimmed() : "";
    QString newFoodDetails = foodDetailsEdit ? foodDetailsEdit->text().trimmed() : "";
    QString newPrice = priceSpinBox ? QString::number(priceSpinBox->value()) : "0";

    if(newFoodType.isEmpty() || newFoodName.isEmpty() || newFoodDetails.isEmpty() || newPrice == "0")
    {
        QMessageBox::warning(this, "Input Error", "Please fill all fields with valid values!");
        return;
    }
    
    // Remove old entry
    if(menu_list.contains(selectedFoodType) && menu_list[selectedFoodType].contains(selectedFoodName))
    {
        menu_list[selectedFoodType].remove(selectedFoodName);
        
        // Remove empty food type
        if(menu_list[selectedFoodType].isEmpty())
        {
            menu_list.remove(selectedFoodType);
        }
    }
    
    // Add new entry
    QPair<QString, QString> food(newFoodDetails, newPrice);
    menu_list[newFoodType][newFoodName] = food;
    
    save_menu_to_database();
    refresh_menu_display();
    clear_form();
    
    QMessageBox::information(this, "Success", "Food item updated successfully!");
}

void menu_restaurant::on_deleteFoodButton_clicked()
{
    if(selectedItemIndex == -1)
    {
        QMessageBox::warning(this, "Selection Error", "Please select a food item to delete!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        "Are you sure you want to delete this food item?", 
        QMessageBox::Yes | QMessageBox::No);
    
    if(reply == QMessageBox::Yes)
    {
        if(menu_list.contains(selectedFoodType) && menu_list[selectedFoodType].contains(selectedFoodName))
        {
            menu_list[selectedFoodType].remove(selectedFoodName);
            
            // Remove empty food type
            if(menu_list[selectedFoodType].isEmpty())
            {
                menu_list.remove(selectedFoodType);
            }
        }
        
        save_menu_to_database();
        refresh_menu_display();
        clear_form();
        
        QMessageBox::information(this, "Success", "Food item deleted successfully!");
    }
}

void menu_restaurant::on_clearFormButton_clicked()
{
    clear_form();
}

void menu_restaurant::on_menuItem_selected()
{
    QTableWidget *menuTableWidget = ui->menuDisplayGroup->findChild<QTableWidget*>("menuTableWidget");
    QLineEdit *foodTypeEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodTypeEdit");
    QLineEdit *foodNameEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodNameEdit");
    QLineEdit *foodDetailsEdit = ui->foodManagementGroup->findChild<QLineEdit*>("foodDetailsEdit");
    QSpinBox *priceSpinBox = ui->foodManagementGroup->findChild<QSpinBox*>("priceSpinBox");

    QList<QTableWidgetItem*> selectedItems = menuTableWidget->selectedItems();
    if(selectedItems.isEmpty()) return;
    int row = selectedItems.first()->row();
    if(foodTypeEdit) foodTypeEdit->setText(menuTableWidget->item(row, 0)->text());
    if(foodNameEdit) foodNameEdit->setText(menuTableWidget->item(row, 1)->text());
    if(foodDetailsEdit) foodDetailsEdit->setText(menuTableWidget->item(row, 2)->text());
    if(priceSpinBox) priceSpinBox->setValue(menuTableWidget->item(row, 3)->text().toInt());
    selectedItemIndex = row;
    selectedFoodType = menuTableWidget->item(row, 0)->text();
    selectedFoodName = menuTableWidget->item(row, 1)->text();
}

void menu_restaurant::click_shopping_basket_button()
{
    message = "shop";
    emit click_server();
}

void menu_restaurant::on_shopping_basket_button_clicked()
{
    shopping_basket *sb = new shopping_basket(currentRestaurantUsername);
    sb->setAttribute(Qt::WA_DeleteOnClose);
    sb->showMaximized();
    this->close();
}

void menu_restaurant::on_profile_button_clicked()
{
    restaurant_auth *ra = new restaurant_auth(currentRestaurantUsername);
    ra->setAttribute(Qt::WA_DeleteOnClose);
    ra->showMaximized();
    this->close();
}

void menu_restaurant::on_logout_button_clicked()
{
    MainWindow *mw = new MainWindow();
    mw->setAttribute(Qt::WA_DeleteOnClose);
    mw->show();
    this->close();
}

void menu_restaurant::send_message()
{
    if(socket.state() == QTcpSocket::ConnectedState)
    {
        QByteArray byte(this->message.c_str(),this->message.length());
        if(socket.write(byte) > 0)
        {
            if(!socket.waitForBytesWritten(1000))
            {
                QMessageBox::information(this,"not success","message not send!!!");
            }
        }
    }
}

void menu_restaurant::receive_message()
{
    if(socket.state() == QTcpSocket::ConnectedState)
    {
        if(socket.waitForReadyRead(-1))
        {
            QByteArray byte = socket.readAll();
            this->message = std::string(byte.constData(),byte.length());
            if(message == "start back")
            {
                message = "";
            }
            else if(message == "start shop")
            {
                message = "";
                emit click_shop();
            }
        }
    }
}

void menu_restaurant::load_orders()
{
    if (currentRestaurantId == -1) return;

    populate_orders_table();
    ui->ordersTableWidget->setRowCount(0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(
        "SELECT o.id, u.username, o.total_amount, o.order_status, o.created_at "
        "FROM orders o "
        "JOIN users u ON o.customer_id = u.id "
        "WHERE o.restaurant_id = ? "
        "ORDER BY o.created_at DESC"
    );
    query.addBindValue(currentRestaurantId);

    if (query.exec()) {
        int row = 0;
        while (query.next()) {
            ui->ordersTableWidget->insertRow(row);
            // Order ID (Column 0) - Store it as user data
            QTableWidgetItem *idItem = new QTableWidgetItem();
            idItem->setData(Qt::UserRole, query.value(0));
            ui->ordersTableWidget->setItem(row, 0, idItem);

            // Customer (Column 1)
            ui->ordersTableWidget->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            // Total (Column 2)
            ui->ordersTableWidget->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
            // Status (Column 3)
            ui->ordersTableWidget->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
            // Date (Column 4)
            ui->ordersTableWidget->setItem(row, 4, new QTableWidgetItem(query.value(4).toDateTime().toString()));
            row++;
        }
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to load orders: " + query.lastError().text());
    }
}

void menu_restaurant::populate_orders_table()
{
    ui->ordersTableWidget->setColumnCount(5);
    ui->ordersTableWidget->setHorizontalHeaderLabels({"ID", "Customer", "Total Price", "Status", "Date"});
    ui->ordersTableWidget->setColumnHidden(0, true); // Hide the ID column
    ui->ordersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ordersTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ordersTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void menu_restaurant::on_refreshOrdersButton_clicked()
{
    load_orders();
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
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) return;

        QSqlQuery query(db);
        query.prepare("UPDATE orders SET order_status = ? WHERE id = ?");
        query.addBindValue(newStatus);
        query.addBindValue(orderId);

        if (query.exec()) {
            QMessageBox::information(this, "Success", "Order status updated successfully!");
            load_orders(); // Refresh the table
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to update order status: " + query.lastError().text());
        }
    }
}

