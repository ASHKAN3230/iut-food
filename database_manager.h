#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QMap>
#include <QPair>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* getInstance();
    
    bool initializeDatabase();
    bool isDatabaseConnected();
    
    // Menu operations
    bool createMenuTable();
    bool addMenuItem(int restaurantId, const QString& foodType, const QString& foodName, 
                    const QString& foodDetails, int price);
    bool updateMenuItem(int itemId, const QString& foodType, const QString& foodName, 
                       const QString& foodDetails, int price);
    bool deleteMenuItem(int itemId);
    QList<QMap<QString, QVariant>> getMenuItems(int restaurantId = -1);
    int getMenuItemCount();
    
    // Restaurant operations
    bool createRestaurantTable();
    bool addRestaurant(const QString& name, const QString& type, const QString& location, 
                      int minPrice, int maxPrice, const QString& description = "");
    bool updateRestaurant(int restaurantId, const QString& name, const QString& type, 
                         const QString& location, int minPrice, int maxPrice, const QString& description = "");
    bool getRestaurantInfo(int restaurantId, QMap<QString, QVariant>& restaurantInfo);
    bool getRestaurantByUsername(const QString& username, QMap<QString, QVariant>& restaurantInfo);
    QList<QMap<QString, QVariant>> getRestaurants();
    int getRestaurantCount();
    
    // Order operations
    bool createOrderTable();
    bool createOrderItemTable();
    bool addOrder(int customerId, int restaurantId, int totalAmount);
    bool addOrderItem(int orderId, int menuItemId, int quantity, int price);
    
    // User operations
    bool createUserTable();
    bool addUser(const QString& username, const QString& password, const QString& userType);
    bool authenticateUser(const QString& username, const QString& password, QString& userType);
    bool getUserInfo(const QString& username, QMap<QString, QVariant>& userInfo);

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    static DatabaseManager* instance;
    QSqlDatabase db;
    
    bool executeQuery(const QString& query);
    bool executeQuery(QSqlQuery& query);
};

#endif // DATABASE_MANAGER_H 