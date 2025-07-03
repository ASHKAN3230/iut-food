#include "network_manager.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>
#include <QUrlQuery>

NetworkManager* NetworkManager::instance = nullptr;

NetworkManager* NetworkManager::getInstance()
{
    if (instance == nullptr) {
        instance = new NetworkManager();
    }
    return instance;
}

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , networkManager(new QNetworkAccessManager(this))
    , serverUrl("http://localhost:8080")
{
    connect(networkManager, &QNetworkAccessManager::finished, 
            this, [this](QNetworkReply *reply) {
        handleResponse(reply, reply->property("operation").toString());
    });
}

NetworkManager::~NetworkManager()
{
    if (instance == this) {
        instance = nullptr;
    }
}

void NetworkManager::setServerUrl(const QString &url)
{
    serverUrl = url;
}

QString NetworkManager::getServerUrl() const
{
    return serverUrl;
}

void NetworkManager::login(const QString &username, const QString &password)
{
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    
    sendRequest("/api/auth/login", "POST", data);
}

void NetworkManager::registerUser(const QString &username, const QString &password, const QString &userType)
{
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    data["userType"] = userType;
    
    sendRequest("/api/auth/register", "POST", data);
}

void NetworkManager::getRestaurants()
{
    sendRequest("/api/restaurants", "GET");
}

void NetworkManager::getRestaurantMenu(int restaurantId)
{
    QString endpoint = QString("/api/restaurants/%1/menu").arg(restaurantId);
    sendRequest(endpoint, "GET");
}

void NetworkManager::addMenuItem(int restaurantId, const QString &foodType, const QString &foodName, 
                                const QString &foodDetails, int price)
{
    QJsonObject data;
    data["restaurantId"] = QString::number(restaurantId);
    data["foodType"] = foodType;
    data["foodName"] = foodName;
    data["foodDetails"] = foodDetails;
    data["price"] = price;
    
    sendRequest("/api/menu", "POST", data);
}

void NetworkManager::updateMenuItem(int itemId, const QString &foodType, const QString &foodName, 
                                   const QString &foodDetails, int price)
{
    QJsonObject data;
    data["foodType"] = foodType;
    data["foodName"] = foodName;
    data["foodDetails"] = foodDetails;
    data["price"] = price;
    
    QString endpoint = QString("/api/menu/%1").arg(itemId);
    sendRequest(endpoint, "PUT", data);
}

void NetworkManager::deleteMenuItem(int itemId)
{
    QString endpoint = QString("/api/menu/%1").arg(itemId);
    sendRequest(endpoint, "DELETE");
}

void NetworkManager::createOrder(int customerId, int restaurantId, const QJsonArray &items, int totalAmount)
{
    QJsonObject data;
    data["customerId"] = QString::number(customerId);
    data["restaurantId"] = QString::number(restaurantId);
    data["items"] = items;
    data["totalAmount"] = totalAmount;
    
    sendRequest("/api/orders", "POST", data);
}

void NetworkManager::getOrders(const QString &userId, const QString &userType)
{
    QString endpoint = QString("/api/orders?userId=%1&userType=%2").arg(userId).arg(userType);
    sendRequest(endpoint, "GET");
}

void NetworkManager::updateOrderStatus(int orderId, const QString &status)
{
    QJsonObject data;
    data["status"] = status;
    
    QString endpoint = QString("/api/orders/%1").arg(orderId);
    sendRequest(endpoint, "PUT", data);
}

void NetworkManager::checkServerHealth()
{
    sendRequest("/api/health", "GET");
}

void NetworkManager::setUserRestaurant(int userId, int restaurantId)
{
    QJsonObject data;
    data["userId"] = userId;
    data["restaurantId"] = restaurantId;
    sendRequest("/api/users/set-restaurant", "POST", data);
}

void NetworkManager::createRestaurant(const QJsonObject &data)
{
    sendRequest("/api/restaurants/create", "POST", data);
}

void NetworkManager::forgotPassword(const QString &username, const QString &newPassword) {
    QJsonObject data;
    data["username"] = username;
    data["password"] = newPassword;
    sendRequest("/api/forgot-password", "POST", data);
}

void NetworkManager::getPendingAuthApplications() {
    sendRequest("/api/restaurants/pending-auth", "GET");
}

void NetworkManager::getAllOrdersAndUsers() {
    sendRequest("/api/debug/orders", "GET");
}

void NetworkManager::approveAuthApplication(int applicationId) {
    QJsonObject data;
    data["applicationId"] = applicationId;
    data["is_auth"] = 1;
    sendRequest("/api/restaurants/auth-status", "POST", data);
}

void NetworkManager::denyAuthApplication(int applicationId) {
    QJsonObject data;
    data["applicationId"] = applicationId;
    data["is_auth"] = 0;
    sendRequest("/api/restaurants/auth-status", "POST", data);
}

void NetworkManager::removeRestaurant(int restaurantId)
{
    QString endpoint = QString("/api/restaurants/%1").arg(restaurantId);
    sendRequest(endpoint, "DELETE");
}

void NetworkManager::removeUser(int userId)
{
    QString endpoint = QString("/api/users/%1").arg(userId);
    sendRequest(endpoint, "DELETE");
}

void NetworkManager::sendRequest(const QString &endpoint, const QString &method, const QJsonObject &data)
{
    QNetworkRequest request = createRequest(endpoint);
    QNetworkReply *reply = nullptr;
    
    if (method == "GET") {
        reply = networkManager->get(request);
    } else if (method == "POST") {
        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson();
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        reply = networkManager->post(request, jsonData);
    } else if (method == "PUT") {
        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson();
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        reply = networkManager->put(request, jsonData);
    } else if (method == "DELETE") {
        reply = networkManager->deleteResource(request);
    }
    
    if (reply) {
        // Store the operation type for response handling
        reply->setProperty("operation", endpoint);
        reply->setProperty("method", method);
    }
}

QNetworkRequest NetworkManager::createRequest(const QString &endpoint)
{
    QUrl url(serverUrl + endpoint);
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "IUT-Food-Client/1.0");
    
    return request;
}

void NetworkManager::handleResponse(QNetworkReply *reply, const QString &operation)
{
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("Network error: %1").arg(reply->errorString());
        qWarning() << errorMsg;
        emit networkError(errorMsg);
        if (operation == "/api/restaurants/create") {
            emit restaurantCreated(false);
        }
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        QString errorMsg = QString("JSON parse error: %1").arg(parseError.errorString());
        qWarning() << errorMsg;
        emit networkError(errorMsg);
        if (operation == "/api/restaurants/create") {
            emit restaurantCreated(false);
        }
        return;
    }

    // Handle direct array response for /api/restaurants
    if (operation == "/api/restaurants" && doc.isArray()) {
        QJsonArray restaurants = doc.array();
        emit restaurantsReceived(restaurants);
        return;
    }

    // Handle direct array response for /api/restaurants/:id/menu
    if (operation.startsWith("/api/restaurants/") && operation.endsWith("/menu")) {
        if (doc.isArray()) {
            emit menuReceived(doc.array());
            return;
        }
        handleMenuResponse(doc.object());
        return;
    }

    QJsonObject response = doc.object();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    qDebug() << "Response for" << operation << ":" << statusCode << response;
    
    // Handle different endpoints
    if (operation == "/api/auth/login") {
        handleLoginResponse(response);
    } else if (operation == "/api/auth/register") {
        handleRegisterResponse(response);
    } else if (operation == "/api/restaurants") {
        handleRestaurantsResponse(response);
    } else if (operation.startsWith("/api/restaurants/") && operation.endsWith("/menu")) {
        handleMenuResponse(response);
    } else if (operation == "/api/menu" && reply->property("method").toString() == "POST") {
        handleMenuItemAddedResponse(response);
    } else if (operation.startsWith("/api/menu/") && reply->property("method").toString() == "PUT") {
        handleMenuItemUpdatedResponse(response);
    } else if (operation.startsWith("/api/menu/") && reply->property("method").toString() == "DELETE") {
        handleMenuItemDeletedResponse(response);
    } else if (operation == "/api/orders" && reply->property("method").toString() == "POST") {
        handleOrderResponse(response);
    } else if (operation.startsWith("/api/orders") && reply->property("method").toString() == "GET") {
        handleOrdersResponse(doc);
    } else if (operation.startsWith("/api/orders/") && reply->property("method").toString() == "PUT") {
        handleOrderStatusResponse(response);
    } else if (operation == "/api/health") {
        handleHealthResponse(response);
    } else if (operation == "/api/forgot-password" && reply->property("method").toString() == "POST") {
        if (response.contains("error")) {
            emit forgotPasswordFailed(response["error"].toString());
        } else {
            emit forgotPasswordSuccess(response["message"].toString());
        }
        return;
    } else if (operation == "/api/restaurants/pending-auth" && reply->property("method").toString() == "GET") {
        if (doc.isArray()) {
            emit pendingAuthApplicationsReceived(doc.array());
        }
        return;
    } else if (operation == "/api/debug/orders" && reply->property("method").toString() == "GET") {
        if (doc.isObject()) {
            emit allOrdersAndUsersReceived(doc.object());
        }
        return;
    } else if (operation == "/api/restaurants/auth-status" && reply->property("method").toString() == "POST") {
        if (response.contains("error")) {
            emit authApplicationFailed(response["error"].toString());
        } else if (response.contains("message")) {
            QString msg = response["message"].toString();
            if (msg.contains("approved", Qt::CaseInsensitive)) {
                emit authApplicationApproved(msg);
            } else if (msg.contains("denied", Qt::CaseInsensitive)) {
                emit authApplicationDenied(msg);
            }
        }
        return;
    }
    // Handle restaurant creation response
    if (operation == "/api/restaurants/create") {
        if (response.contains("error")) {
            emit restaurantCreated(false);
        } else {
            emit restaurantCreated(true);
        }
    }
}

void NetworkManager::handleLoginResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit loginFailed(response["error"].toString());
    } else {
        emit loginSuccess(response);
    }
}

void NetworkManager::handleRegisterResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit registerFailed(response["error"].toString());
    } else {
        emit registerSuccess(response["message"].toString());
    }
}

void NetworkManager::handleRestaurantsResponse(const QJsonObject &response)
{
    // The server should return an array of restaurants
    // For now, we'll assume it's directly an array
    QJsonArray restaurants;
    if (response.contains("restaurants")) {
        restaurants = response["restaurants"].toArray();
    } else {
        // If response is directly an array
        QJsonDocument doc = QJsonDocument::fromJson(response.toVariantMap().value("data").toByteArray());
        if (doc.isArray()) {
            restaurants = doc.array();
        }
    }
    
    emit restaurantsReceived(restaurants);
}

void NetworkManager::handleMenuResponse(const QJsonObject &response)
{
    QJsonArray menu;
    if (response.contains("menu")) {
        menu = response["menu"].toArray();
    } else {
        // If response is directly an array
        QJsonDocument doc = QJsonDocument::fromJson(response.toVariantMap().value("data").toByteArray());
        if (doc.isArray()) {
            menu = doc.array();
        }
    }
    
    emit menuReceived(menu);
}

void NetworkManager::handleMenuItemAddedResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit menuItemAddedFailed(response["error"].toString());
    } else {
        emit menuItemAdded(response["message"].toString());
    }
}

void NetworkManager::handleMenuItemUpdatedResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit menuItemUpdatedFailed(response["error"].toString());
    } else {
        emit menuItemUpdated(response["message"].toString());
    }
}

void NetworkManager::handleMenuItemDeletedResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit menuItemDeletedFailed(response["error"].toString());
    } else {
        emit menuItemDeleted(response["message"].toString());
    }
}

void NetworkManager::handleOrderResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit orderCreationFailed(response["error"].toString());
    } else {
        emit orderCreated(response["message"].toString());
    }
}

void NetworkManager::handleOrdersResponse(const QJsonDocument &doc)
{
    QJsonArray orders;
    if (doc.isArray()) {
        orders = doc.array();
    } else if (doc.isObject()) {
        QJsonObject response = doc.object();
        if (response.contains("orders")) {
            orders = response["orders"].toArray();
        }
    }
    emit ordersReceived(orders);
}

void NetworkManager::handleOrderStatusResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit orderStatusUpdateFailed(response["error"].toString());
    } else {
        emit orderStatusUpdated(response["message"].toString());
    }
}

void NetworkManager::handleHealthResponse(const QJsonObject &response)
{
    if (response.contains("error")) {
        emit serverHealthFailed(response["error"].toString());
    } else {
        emit serverHealthOk(response);
    }
}