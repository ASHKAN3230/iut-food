#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QString>

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static NetworkManager* getInstance();
    
    // Configuration
    void setServerUrl(const QString &url);
    QString getServerUrl() const;
    
    // Authentication
    void login(const QString &username, const QString &password);
    void registerUser(const QString &username, const QString &password, const QString &userType);
    
    // Restaurant operations
    void getRestaurants();
    void getRestaurantMenu(int restaurantId);
    
    // Menu operations
    void addMenuItem(int restaurantId, const QString &foodType, const QString &foodName, 
                    const QString &foodDetails, int price);
    void updateMenuItem(int itemId, const QString &foodType, const QString &foodName, 
                       const QString &foodDetails, int price);
    void deleteMenuItem(int itemId);
    
    // Order operations
    void createOrder(int customerId, int restaurantId, const QJsonArray &items, int totalAmount);
    void getOrders(const QString &userId, const QString &userType);
    void updateOrderStatus(int orderId, const QString &status);
    
    // Health check
    void checkServerHealth();

    void setUserRestaurant(int userId, int restaurantId);

    void createRestaurant(const QJsonObject &data);

signals:
    // Authentication signals
    void loginSuccess(const QJsonObject &userInfo);
    void loginFailed(const QString &error);
    void registerSuccess(const QString &message);
    void registerFailed(const QString &error);
    
    // Restaurant signals
    void restaurantsReceived(const QJsonArray &restaurants);
    void menuReceived(const QJsonArray &menu);
    
    // Menu operation signals
    void menuItemAdded(const QString &message);
    void menuItemAddedFailed(const QString &error);
    void menuItemUpdated(const QString &message);
    void menuItemUpdatedFailed(const QString &error);
    void menuItemDeleted(const QString &message);
    void menuItemDeletedFailed(const QString &error);
    
    // Order signals
    void orderCreated(const QString &message);
    void orderCreationFailed(const QString &error);
    void ordersReceived(const QJsonArray &orders);
    void orderStatusUpdated(const QString &message);
    void orderStatusUpdateFailed(const QString &error);
    
    // System signals
    void serverHealthOk(const QJsonObject &status);
    void serverHealthFailed(const QString &error);
    void networkError(const QString &error);
    void restaurantCreated(bool success);

private:
    NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();
    
    static NetworkManager* instance;
    QNetworkAccessManager* networkManager;
    QString serverUrl;
    
    // Helper methods
    void sendRequest(const QString &endpoint, const QString &method, const QJsonObject &data = QJsonObject());
    void handleResponse(QNetworkReply *reply, const QString &operation);
    QNetworkRequest createRequest(const QString &endpoint);
    
    // Response handlers
    void handleLoginResponse(const QJsonObject &response);
    void handleRegisterResponse(const QJsonObject &response);
    void handleRestaurantsResponse(const QJsonObject &response);
    void handleMenuResponse(const QJsonObject &response);
    void handleMenuItemAddedResponse(const QJsonObject &response);
    void handleMenuItemUpdatedResponse(const QJsonObject &response);
    void handleMenuItemDeletedResponse(const QJsonObject &response);
    void handleOrderResponse(const QJsonObject &response);
    void handleOrdersResponse(const QJsonDocument &doc);
    void handleOrderStatusResponse(const QJsonObject &response);
    void handleHealthResponse(const QJsonObject &response);
};

#endif // NETWORK_MANAGER_H