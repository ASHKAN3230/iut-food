#include "database_manager.h"
#include <QDebug>
#include <QMessageBox>

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager* DatabaseManager::getInstance()
{
    if (instance == nullptr) {
        instance = new DatabaseManager();
    }
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("iut_food.db");
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::initializeDatabase()
{
    if (!db.open()) {
        qDebug() << "Database Error:" << db.lastError().text();
        return false;
    }
    
    // Create all necessary tables
    createUserTable();
    createRestaurantTable();
    createMenuTable();
    createOrderTable();
    createOrderItemTable();
    
    return true;
}

bool DatabaseManager::isDatabaseConnected()
{
    return db.isOpen();
}

bool DatabaseManager::createUserTable()
{
    QString query = "CREATE TABLE IF NOT EXISTS users ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "username TEXT NOT NULL UNIQUE,"
                   "password TEXT NOT NULL,"
                   "user_type TEXT NOT NULL CHECK(user_type IN ('customer', 'manager', 'restaurant')),"
                   "restaurant_id INTEGER,"
                   "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ");";
    return executeQuery(query);
}

bool DatabaseManager::createMenuTable()
{
    QString query = "CREATE TABLE IF NOT EXISTS menu_items ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "restaurant_id INTEGER,"
                   "food_type TEXT NOT NULL,"
                   "food_name TEXT NOT NULL,"
                   "food_details TEXT NOT NULL,"
                   "price INTEGER NOT NULL,"
                   "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ");";
    return executeQuery(query);
}

bool DatabaseManager::createRestaurantTable()
{
    QString query = "CREATE TABLE IF NOT EXISTS restaurants ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "name TEXT NOT NULL,"
                   "type TEXT NOT NULL,"
                   "location TEXT NOT NULL,"
                   "description TEXT,"
                   "min_price INTEGER NOT NULL,"
                   "max_price INTEGER NOT NULL,"
                   "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ");";
    return executeQuery(query);
}

bool DatabaseManager::createOrderTable()
{
    QString query = "CREATE TABLE IF NOT EXISTS orders ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "customer_id INTEGER,"
                   "restaurant_id INTEGER,"
                   "total_amount INTEGER NOT NULL,"
                   "order_status TEXT DEFAULT 'pending',"
                   "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                   ");";
    return executeQuery(query);
}

bool DatabaseManager::createOrderItemTable()
{
    QString query = "CREATE TABLE IF NOT EXISTS order_items ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "order_id INTEGER,"
                   "menu_item_id INTEGER,"
                   "quantity INTEGER NOT NULL,"
                   "price INTEGER NOT NULL,"
                   "FOREIGN KEY (order_id) REFERENCES orders(id),"
                   "FOREIGN KEY (menu_item_id) REFERENCES menu_items(id)"
                   ");";
    return executeQuery(query);
}

bool DatabaseManager::addMenuItem(int restaurantId, const QString& foodType, const QString& foodName, 
                                 const QString& foodDetails, int price)
{
    QSqlQuery query;
    query.prepare("INSERT INTO menu_items (restaurant_id, food_type, food_name, food_details, price) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(restaurantId);
    query.addBindValue(foodType);
    query.addBindValue(foodName);
    query.addBindValue(foodDetails);
    query.addBindValue(price);
    
    return executeQuery(query);
}

bool DatabaseManager::updateMenuItem(int itemId, const QString& foodType, const QString& foodName, 
                                   const QString& foodDetails, int price)
{
    QSqlQuery query;
    query.prepare("UPDATE menu_items SET food_type = ?, food_name = ?, food_details = ?, price = ? "
                  "WHERE id = ?");
    query.addBindValue(foodType);
    query.addBindValue(foodName);
    query.addBindValue(foodDetails);
    query.addBindValue(price);
    query.addBindValue(itemId);
    
    return executeQuery(query);
}

bool DatabaseManager::deleteMenuItem(int itemId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM menu_items WHERE id = ?");
    query.addBindValue(itemId);
    
    return executeQuery(query);
}

QList<QMap<QString, QVariant>> DatabaseManager::getMenuItems(int restaurantId)
{
    QList<QMap<QString, QVariant>> items;
    
    QSqlQuery query;
    if (restaurantId == -1) {
        query.prepare("SELECT id, restaurant_id, food_type, food_name, food_details, price FROM menu_items ORDER BY food_type, food_name");
    } else {
        query.prepare("SELECT id, restaurant_id, food_type, food_name, food_details, price FROM menu_items WHERE restaurant_id = ? ORDER BY food_type, food_name");
        query.addBindValue(restaurantId);
    }
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> item;
            item["id"] = query.value(0);
            item["restaurant_id"] = query.value(1);
            item["food_type"] = query.value(2);
            item["food_name"] = query.value(3);
            item["food_details"] = query.value(4);
            item["price"] = query.value(5);
            items.append(item);
        }
    }
    
    return items;
}

int DatabaseManager::getMenuItemCount()
{
    QSqlQuery query("SELECT COUNT(*) FROM menu_items");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool DatabaseManager::addRestaurant(const QString& name, const QString& type, const QString& location, 
                                  int minPrice, int maxPrice, const QString& description)
{
    QSqlQuery query;
    query.prepare("INSERT INTO restaurants (name, type, location, description, min_price, max_price) "
                  "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(type);
    query.addBindValue(location);
    query.addBindValue(description);
    query.addBindValue(minPrice);
    query.addBindValue(maxPrice);
    
    return executeQuery(query);
}

bool DatabaseManager::updateRestaurant(int restaurantId, const QString& name, const QString& type, 
                                     const QString& location, int minPrice, int maxPrice, const QString& description)
{
    QSqlQuery query;
    query.prepare("UPDATE restaurants SET name = ?, type = ?, location = ?, description = ?, min_price = ?, max_price = ? "
                  "WHERE id = ?");
    query.addBindValue(name);
    query.addBindValue(type);
    query.addBindValue(location);
    query.addBindValue(description);
    query.addBindValue(minPrice);
    query.addBindValue(maxPrice);
    query.addBindValue(restaurantId);
    
    return executeQuery(query);
}

bool DatabaseManager::getRestaurantInfo(int restaurantId, QMap<QString, QVariant>& restaurantInfo)
{
    QSqlQuery query;
    query.prepare("SELECT id, name, type, location, description, min_price, max_price FROM restaurants WHERE id = ?");
    query.addBindValue(restaurantId);
    
    if (query.exec() && query.next()) {
        restaurantInfo["id"] = query.value(0);
        restaurantInfo["name"] = query.value(1);
        restaurantInfo["type"] = query.value(2);
        restaurantInfo["location"] = query.value(3);
        restaurantInfo["description"] = query.value(4);
        restaurantInfo["min_price"] = query.value(5);
        restaurantInfo["max_price"] = query.value(6);
        return true;
    }
    return false;
}

bool DatabaseManager::getRestaurantByUsername(const QString& username, QMap<QString, QVariant>& restaurantInfo)
{
    QSqlQuery query;
    query.prepare("SELECT r.id, r.name, r.type, r.location, r.description, r.min_price, r.max_price "
                  "FROM restaurants r "
                  "JOIN users u ON r.id = u.restaurant_id "
                  "WHERE u.username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        restaurantInfo["id"] = query.value(0);
        restaurantInfo["name"] = query.value(1);
        restaurantInfo["type"] = query.value(2);
        restaurantInfo["location"] = query.value(3);
        restaurantInfo["description"] = query.value(4);
        restaurantInfo["min_price"] = query.value(5);
        restaurantInfo["max_price"] = query.value(6);
        return true;
    }
    return false;
}

QList<QMap<QString, QVariant>> DatabaseManager::getRestaurants()
{
    QList<QMap<QString, QVariant>> restaurants;
    
    QSqlQuery query("SELECT id, name, type, location, description, min_price, max_price FROM restaurants ORDER BY name");
    
    if (query.exec()) {
        while (query.next()) {
            QMap<QString, QVariant> restaurant;
            restaurant["id"] = query.value(0);
            restaurant["name"] = query.value(1);
            restaurant["type"] = query.value(2);
            restaurant["location"] = query.value(3);
            restaurant["description"] = query.value(4);
            restaurant["min_price"] = query.value(5);
            restaurant["max_price"] = query.value(6);
            restaurants.append(restaurant);
        }
    }
    
    return restaurants;
}

int DatabaseManager::getRestaurantCount()
{
    QSqlQuery query("SELECT COUNT(*) FROM restaurants");
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool DatabaseManager::addUser(const QString& username, const QString& password, const QString& userType)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, user_type) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(userType);
    
    return executeQuery(query);
}

bool DatabaseManager::authenticateUser(const QString& username, const QString& password, QString& userType)
{
    QSqlQuery query;
    query.prepare("SELECT user_type FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    
    if (query.exec() && query.next()) {
        userType = query.value(0).toString();
        return true;
    }
    return false;
}

bool DatabaseManager::getUserInfo(const QString& username, QMap<QString, QVariant>& userInfo)
{
    QSqlQuery query;
    query.prepare("SELECT id, username, user_type, restaurant_id FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        userInfo["id"] = query.value(0);
        userInfo["username"] = query.value(1);
        userInfo["user_type"] = query.value(2);
        userInfo["restaurant_id"] = query.value(3);
        return true;
    }
    return false;
}

bool DatabaseManager::addOrder(int customerId, int restaurantId, int totalAmount)
{
    QSqlQuery query;
    query.prepare("INSERT INTO orders (customer_id, restaurant_id, total_amount) VALUES (?, ?, ?)");
    query.addBindValue(customerId);
    query.addBindValue(restaurantId);
    query.addBindValue(totalAmount);
    
    return executeQuery(query);
}

bool DatabaseManager::addOrderItem(int orderId, int menuItemId, int quantity, int price)
{
    QSqlQuery query;
    query.prepare("INSERT INTO order_items (order_id, menu_item_id, quantity, price) VALUES (?, ?, ?, ?)");
    query.addBindValue(orderId);
    query.addBindValue(menuItemId);
    query.addBindValue(quantity);
    query.addBindValue(price);
    
    return executeQuery(query);
}

bool DatabaseManager::executeQuery(const QString& query)
{
    QSqlQuery sqlQuery;
    return sqlQuery.exec(query);
}

bool DatabaseManager::executeQuery(QSqlQuery& query)
{
    return query.exec();
} 