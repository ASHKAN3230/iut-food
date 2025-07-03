#include "manager_dashboard.h"
#include "ui_manager_dashboard.h"
#include "network_manager.h"
#include <QTabWidget>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QSizePolicy>

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
    connect(NetworkManager::getInstance(), &NetworkManager::authApplicationApproved, this, [this](const QString &msg) {
        QMessageBox::information(this, "Approved", msg);
        NetworkManager::getInstance()->getPendingAuthApplications();
    });
    connect(NetworkManager::getInstance(), &NetworkManager::authApplicationDenied, this, [this](const QString &msg) {
        QMessageBox::information(this, "Denied", msg);
        NetworkManager::getInstance()->getPendingAuthApplications();
    });
    connect(NetworkManager::getInstance(), &NetworkManager::authApplicationFailed, this, [this](const QString &err) {
        QMessageBox::warning(this, "Error", err);
    });

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

    // Fetch data for the initially selected tab
    int initialTab = ui->tabWidget->currentIndex();
    QString tabName = ui->tabWidget->widget(initialTab)->objectName();
    if (tabName == "authApplicationsTab") {
        NetworkManager::getInstance()->getPendingAuthApplications();
    } else if (tabName == "ordersAnalysisTab") {
        NetworkManager::getInstance()->getAllOrdersAndUsers();
    } else if (tabName == "restaurantsTab") {
        NetworkManager::getInstance()->getRestaurants();
    } else if (tabName == "usersTab") {
        NetworkManager::getInstance()->getAllOrdersAndUsers();
    }
}

manager_dashboard::~manager_dashboard()
{
    delete ui;
}

void manager_dashboard::initRestaurantsTab()
{
    auto table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({"ID", "Name", "Type", "Location", "Description", "Remove"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->restaurantsTab->layout()->addWidget(table);
    restaurantsTable = table;
}

void manager_dashboard::initUsersTab()
{
    auto table = new QTableWidget(0, 5, this);
    table->setHorizontalHeaderLabels({"ID", "Username", "Type", "RestaurantID", "Remove"});
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
    auto vbox = qobject_cast<QVBoxLayout*>(ui->ordersAnalysisTab->layout());
    summaryLabel = new QLabel("", this);
    vbox->insertWidget(0, summaryLabel);
    auto table = new QTableWidget(0, 6, this);
    table->setHorizontalHeaderLabels({"OrderID", "Customer", "Restaurant", "Amount", "Status", "CreatedAt"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    vbox->addWidget(table);
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
        QPushButton *removeBtn = new QPushButton("Remove");
        removeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        QSize sz = removeBtn->sizeHint();
        removeBtn->setMinimumSize(sz);
        removeBtn->setMaximumSize(sz);
        int restaurantId = obj["id"].toInt();
        connect(removeBtn, &QPushButton::clicked, this, [this, restaurantId]() {
            if (QMessageBox::question(this, "Remove Restaurant", "Are you sure you want to remove this restaurant?") == QMessageBox::Yes) {
                NetworkManager* net = NetworkManager::getInstance();
                connect(net, &NetworkManager::restaurantsReceived, this, [this](const QJsonArray &){
                    // Disconnect after first refresh to avoid duplicate refreshes
                    disconnect(NetworkManager::getInstance(), &NetworkManager::restaurantsReceived, this, nullptr);
                });
                net->removeRestaurant(restaurantId);
                net->getRestaurants();
            }
        });
        restaurantsTable->setCellWidget(row, 5, removeBtn);
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
            QPushButton *removeBtn = new QPushButton("Remove");
            removeBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            QSize sz = removeBtn->sizeHint();
            removeBtn->setMinimumSize(sz);
            removeBtn->setMaximumSize(sz);
            int userId = u["id"].toInt();
            connect(removeBtn, &QPushButton::clicked, this, [this, userId]() {
                if (QMessageBox::question(this, "Remove User", "Are you sure you want to remove this user?") == QMessageBox::Yes) {
                    NetworkManager* net = NetworkManager::getInstance();
                    connect(net, &NetworkManager::allOrdersAndUsersReceived, this, [this](const QJsonObject &){
                        // Disconnect after first refresh to avoid duplicate refreshes
                        disconnect(NetworkManager::getInstance(), &NetworkManager::allOrdersAndUsersReceived, this, nullptr);
                    });
                    net->removeUser(userId);
                    net->getAllOrdersAndUsers();
                }
            });
            usersTable->setCellWidget(row, 4, removeBtn);
            row++;
        }
    }
    // Orders Table
    ordersTable->setRowCount(0);
    int totalOrders = 0;
    int totalRevenue = 0;
    if (data.contains("orders")) {
        QJsonArray orders = data["orders"].toArray();
        int row = 0;
        totalOrders = orders.size();
        for (const QJsonValue &val : orders) {
            QJsonObject o = val.toObject();
            ordersTable->insertRow(row);
            ordersTable->setItem(row, 0, new QTableWidgetItem(QString::number(o["id"].toInt())));
            ordersTable->setItem(row, 1, new QTableWidgetItem(o["customerName"].toString()));
            ordersTable->setItem(row, 2, new QTableWidgetItem(o["restaurantName"].toString()));
            ordersTable->setItem(row, 3, new QTableWidgetItem(QString::number(o["totalAmount"].toInt())));
            ordersTable->setItem(row, 4, new QTableWidgetItem(o["status"].toString()));
            ordersTable->setItem(row, 5, new QTableWidgetItem(o["createdAt"].toString()));
            totalRevenue += o["totalAmount"].toInt();
            row++;
        }
    }
    int totalUsers = 0;
    if (data.contains("users")) {
        totalUsers = data["users"].toArray().size();
    }
    if (summaryLabel) {
        summaryLabel->setText(QString("Total Orders: %1 | Total Users: %2 | Total Revenue: %3 Tooman")
            .arg(totalOrders).arg(totalUsers).arg(totalRevenue));
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
            NetworkManager::getInstance()->approveAuthApplication(obj["id"].toInt());
        });
        authAppsTable->setCellWidget(row, 5, approveBtn);
        // Deny button
        QPushButton *denyBtn = new QPushButton("Deny");
        connect(denyBtn, &QPushButton::clicked, this, [this, obj]() {
            NetworkManager::getInstance()->denyAuthApplication(obj["id"].toInt());
        });
        authAppsTable->setCellWidget(row, 6, denyBtn);
        row++;
    }
} 
