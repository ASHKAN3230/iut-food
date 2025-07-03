#include "manager_dashboard.h"
#include "ui_manager_dashboard.h"
#include "network_manager.h"
#include <QTabWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>

manager_dashboard::manager_dashboard(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::manager_dashboard)
{
    ui->setupUi(this);
    currentManagerUsername = username;
    initRestaurantsTab();
    initUsersTab();
    initAuthApplicationsTab();
    initOrdersAnalysisTab();

    // Connect signals for API data
    connect(NetworkManager::getInstance(), &NetworkManager::restaurantsReceived, this, &manager_dashboard::onRestaurantsReceived);
    connect(NetworkManager::getInstance(), &NetworkManager::pendingAuthApplicationsReceived, this, &manager_dashboard::onPendingAuthApplicationsReceived);
    connect(NetworkManager::getInstance(), &NetworkManager::allOrdersAndUsersReceived, this, &manager_dashboard::onAllOrdersAndUsersReceived);

    // Connect tab change to trigger API calls
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
        QString tabName = ui->tabWidget->widget(idx)->objectName();
        if (tabName == "authApplicationsTab") {
            NetworkManager::getInstance()->getPendingAuthApplications();
        } else if (tabName == "ordersAnalysisTab") {
            NetworkManager::getInstance()->getAllOrdersAndUsers();
        } else if (tabName == "restaurantsTab") {
            NetworkManager::getInstance()->getRestaurants();
        } else if (tabName == "usersTab") {
            NetworkManager::getInstance()->getAllOrdersAndUsers();
        }
    });
}

manager_dashboard::~manager_dashboard()
{
    delete ui;
}

void manager_dashboard::initRestaurantsTab()
{
    auto table = new QTableWidget(0, 5, this);
    table->setHorizontalHeaderLabels({"ID", "Name", "Type", "Location", "Description"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->restaurantsTab->layout()->addWidget(table);
    restaurantsTable = table;
}

void manager_dashboard::initUsersTab()
{
    auto table = new QTableWidget(0, 4, this);
    table->setHorizontalHeaderLabels({"ID", "Username", "Type", "RestaurantID"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->usersTab->layout()->addWidget(table);
    usersTable = table;
}

void manager_dashboard::initAuthApplicationsTab()
{
    auto table = new QTableWidget(0, 7, this);
    table->setHorizontalHeaderLabels({"ID", "UserID", "Name", "Type", "Location", "Approve", "Deny"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->authApplicationsTab->layout()->addWidget(table);
    authAppsTable = table;
}

void manager_dashboard::initOrdersAnalysisTab()
{
    auto table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({"OrderID", "Customer", "Restaurant", "Amount", "Status", "CreatedAt"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->ordersAnalysisTab->layout()->addWidget(table);
    ordersTable = table;
}

void manager_dashboard::onRestaurantsReceived(const QJsonArray &restaurants)
{
    restaurantsTable->setRowCount(0);
    int row = 0;
    for (const QJsonValue &val : restaurants) {
        QJsonObject obj = val.toObject();
        restaurantsTable->insertRow(row);
        restaurantsTable->setItem(row, 0, new QTableWidgetItem(QString::number(obj["id"].toInt())));
        restaurantsTable->setItem(row, 1, new QTableWidgetItem(obj["name"].toString()));
        restaurantsTable->setItem(row, 2, new QTableWidgetItem(obj["type"].toString()));
        restaurantsTable->setItem(row, 3, new QTableWidgetItem(obj["location"].toString()));
        restaurantsTable->setItem(row, 4, new QTableWidgetItem(obj["description"].toString()));
        row++;
    }
}

void manager_dashboard::onAllOrdersAndUsersReceived(const QJsonObject &data)
{
    // Users Table
    usersTable->setRowCount(0);
    if (data.contains("users")) {
        QJsonArray users = data["users"].toArray();
        int row = 0;
        for (const QJsonValue &val : users) {
            QJsonObject u = val.toObject();
            usersTable->insertRow(row);
            usersTable->setItem(row, 0, new QTableWidgetItem(QString::number(u["id"].toInt())));
            usersTable->setItem(row, 1, new QTableWidgetItem(u["username"].toString()));
            usersTable->setItem(row, 2, new QTableWidgetItem(u["userType"].toString()));
            usersTable->setItem(row, 3, new QTableWidgetItem(u["restaurantId"].isNull() ? "-" : QString::number(u["restaurantId"].toInt())));
            row++;
        }
    }
    // Orders Table
    ordersTable->setRowCount(0);
    if (data.contains("orders")) {
        QJsonArray orders = data["orders"].toArray();
        int row = 0;
        for (const QJsonValue &val : orders) {
            QJsonObject o = val.toObject();
            ordersTable->insertRow(row);
            ordersTable->setItem(row, 0, new QTableWidgetItem(QString::number(o["id"].toInt())));
            ordersTable->setItem(row, 1, new QTableWidgetItem(o["customerName"].toString()));
            ordersTable->setItem(row, 2, new QTableWidgetItem(o["restaurantName"].toString()));
            ordersTable->setItem(row, 3, new QTableWidgetItem(QString::number(o["totalAmount"].toInt())));
            ordersTable->setItem(row, 4, new QTableWidgetItem(o["status"].toString()));
            ordersTable->setItem(row, 5, new QTableWidgetItem(o["createdAt"].toString()));
            row++;
        }
    }
}

void manager_dashboard::onPendingAuthApplicationsReceived(const QJsonArray &apps)
{
    authAppsTable->setRowCount(0);
    int row = 0;
    for (const QJsonValue &val : apps) {
        QJsonObject obj = val.toObject();
        authAppsTable->insertRow(row);
        authAppsTable->setItem(row, 0, new QTableWidgetItem(QString::number(obj["id"].toInt())));
        authAppsTable->setItem(row, 1, new QTableWidgetItem(QString::number(obj["userId"].toInt())));
        authAppsTable->setItem(row, 2, new QTableWidgetItem(obj["name"].toString()));
        authAppsTable->setItem(row, 3, new QTableWidgetItem(obj["type"].toString()));
        authAppsTable->setItem(row, 4, new QTableWidgetItem(obj["location"].toString()));
        // Approve button
        QPushButton *approveBtn = new QPushButton("Approve");
        connect(approveBtn, &QPushButton::clicked, this, [this, obj]() {
            // TODO: Implement approve logic
        });
        authAppsTable->setCellWidget(row, 5, approveBtn);
        // Deny button
        QPushButton *denyBtn = new QPushButton("Deny");
        connect(denyBtn, &QPushButton::clicked, this, [this, obj]() {
            // TODO: Implement deny logic
        });
        authAppsTable->setCellWidget(row, 6, denyBtn);
        row++;
    }
} 
