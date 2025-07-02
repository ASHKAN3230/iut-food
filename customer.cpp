#include "customer.h"
#include "ui_customer.h"
#include "mainwindow.h"
#include "clicklabel.h"
#include "menu_restaurant.h"
#include<QString>
#include<QStringList>
#include<QTextStream>
#include<QFile>
#include<QListWidgetItem>
#include<QMessageBox>
#include<QWidget>
#include<QVBoxLayout>
#include<QLabel>
#include<QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include "network_manager.h"
#include <QSizePolicy>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QComboBox>

customer::customer(const QString &username, int userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::customer)
{
    ui->setupUi(this);
    currentUsername = username;
    currentUserId = userId;
    // Populate filterComboBox with example options
    ui->filterComboBox->addItem("All");
    ui->filterComboBox->addItem("Fast Food");
    ui->filterComboBox->addItem("Cafe");
    ui->filterComboBox->addItem("Restaurant");
    ui->filterComboBox->addItem("Dessert");
    connect(ui->filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &customer::on_filterComboBox_currentIndexChanged);
    fetchAndDisplayRestaurants();
    // Connect tab change to fetch orders
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
        if (ui->tabWidget->widget(idx)->objectName() == "ordersTab") {
            fetchAndDisplayOrders();
        }
    });

    connect(this, &customer::click_search_button, this, &customer::send_message);

    connect(this, &customer::receive_message, this, &customer::on_search_button_clicked);

    connect(this, &customer::click_back_button, this, &customer::send_message);

    connect(this, &customer::receive_message, this, &customer::on_back_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

    this->open_file();

}

customer::~customer()
{
    delete ui;
}

void customer::open_file()
{
    QFile file_8("files/restaurants_list.txt");
    QString line;
    QString str;
    QString key_type;
    QString key_name;
    QVector<QString> vec;
    QStringList strlist;
    int minimum_number;
    int maximum_number;
    int index = 0;
    if(file_8.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file_8);
        // Clear previous results
        QLayoutItem *child;
        while ((child = ui->resultsLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        while(!in.atEnd())
        {
            key_type = in.readLine().trimmed();
            key_name = in.readLine().trimmed();
            str = in.readLine().trimmed();
            strlist = str.split("-");
            vec = strlist.toVector();
            str = in.readLine().trimmed();
            minimum_number = str.toInt();
            str = in.readLine().trimmed();
            maximum_number = str.toInt();
            QPair<int,int> range(minimum_number,maximum_number);
            QPair<QVector<QString>, QPair<int,int>> data(vec,range);
            restaurant_list[key_type][key_name] = data;
            // Create food result widget
            QWidget *foodWidget = new QWidget;
            QVBoxLayout *vbox = new QVBoxLayout(foodWidget);
            QLabel *nameLabel = new QLabel("<b>" + key_name + "</b>");
            QLabel *restaurantLabel = new QLabel("Restaurant: " + key_type);
            QLabel *descLabel = new QLabel("Description: " + strlist.join(", "));
            QLabel *priceLabel = new QLabel("Price: " + QString::number(minimum_number) + " - " + QString::number(maximum_number));
            QPushButton *orderButton = new QPushButton("Order");
            vbox->addWidget(nameLabel);
            vbox->addWidget(restaurantLabel);
            vbox->addWidget(descLabel);
            vbox->addWidget(priceLabel);
            vbox->addWidget(orderButton);
            foodWidget->setLayout(vbox);
            ui->resultsLayout->addWidget(foodWidget);
            // Optionally connect orderButton to a slot
            // connect(orderButton, &QPushButton::clicked, ...);
            line = in.readLine();
            index++;
        }
        file_8.close();
    }
}

void customer::open_next_window(int index)
{
    menu_restaurant *mr = new menu_restaurant(currentUsername);
    mr->setAttribute(Qt::WA_DeleteOnClose);
    mr->showMaximized();
    this->close();
}

void customer::click_search_button()
{

    message = "search";

    emit click_server();

}

void customer::on_search_button_clicked()
{
    QString searchText = ui->search_lineedit->text().trimmed();
    // Clear previous results
    QLayoutItem *child;
    while ((child = ui->resultsLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    int index = 0;
    for(auto i = restaurant_list.begin(); i != restaurant_list.end(); ++i)
    {
        QMap<QString,QPair<QVector<QString>, QPair<int,int>>> &restaurant_name = i.value();
        for(auto j = restaurant_name.begin(); j != restaurant_name.end(); ++j)
        {
            QPair<QVector<QString>, QPair<int,int>> restaurant_position = j.value();
            QString foodName = j.key();
            QString restaurantType = i.key();
            QString description = restaurant_position.first.join(", ");
            int minPrice = restaurant_position.second.first;
            int maxPrice = restaurant_position.second.second;
            // Simple search: match if searchText is in food name, restaurant type, or description
            if (searchText.isEmpty() ||
                foodName.contains(searchText, Qt::CaseInsensitive) ||
                restaurantType.contains(searchText, Qt::CaseInsensitive) ||
                description.contains(searchText, Qt::CaseInsensitive))
            {
                QWidget *foodWidget = new QWidget;
                QVBoxLayout *vbox = new QVBoxLayout(foodWidget);
                QLabel *nameLabel = new QLabel("<b>" + foodName + "</b>");
                QLabel *restaurantLabel = new QLabel("Restaurant: " + restaurantType);
                QLabel *descLabel = new QLabel("Description: " + description);
                QLabel *priceLabel = new QLabel("Price: " + QString::number(minPrice) + " - " + QString::number(maxPrice));
                QPushButton *orderButton = new QPushButton("Order");
                vbox->addWidget(nameLabel);
                vbox->addWidget(restaurantLabel);
                vbox->addWidget(descLabel);
                vbox->addWidget(priceLabel);
                vbox->addWidget(orderButton);
                foodWidget->setLayout(vbox);
                ui->resultsLayout->addWidget(foodWidget);
                // Optionally connect orderButton to a slot
                // connect(orderButton, &QPushButton::clicked, ...);
                index++;
            }
        }
    }
}

void customer::click_back_button()
{

    message = "back";

    emit click_server();

}

void customer::on_back_button_clicked()
{

    MainWindow *mw = new MainWindow();

    mw->setAttribute(Qt::WA_DeleteOnClose);

    mw->show();

    this->close();

}

void customer::send_message()
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

void customer::receive_message()
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

                emit click_back();

            }
            else if(message == "start search")
            {

                message = "";

                emit click_search();

            }

        }

    }

}

void customer::on_filterComboBox_currentIndexChanged(int index) {
    // Example: filter restaurants by type
    QString selectedType = ui->filterComboBox->currentText();
    // You may want to store the last fetched restaurants in a member variable for filtering
    // For now, just refetch and filter in displayRestaurants
    fetchAndDisplayRestaurants();
}

void customer::displayRestaurants(const QJsonArray &restaurants) {
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout*>(ui->resultsContainer->layout());
    if (!vbox) return;
    // Clear previous results
    QLayoutItem *child;
    while ((child = vbox->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    int cardHeight = 110;
    int cardCount = restaurants.size();
    int cardSpacing = 6; // px
    QString selectedType = ui->filterComboBox->currentText();
    int shownCount = 0;
    for (int i = 0; i < cardCount; ++i) {
        const QJsonValue &val = restaurants[i];
        QJsonObject obj = val.toObject();
        if (selectedType != "All" && obj["type"].toString() != selectedType)
            continue;
        QWidget *card = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout(card);
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->setSpacing(0);
        QLabel *nameLabel = new QLabel("<b>" + obj["name"].toString() + "</b>");
        QLabel *typeLabel = new QLabel(obj["type"].toString());
        QLabel *descLabel = new QLabel(obj["description"].toString());
        QLabel *locLabel = new QLabel("Location: " + obj["location"].toString());
        QLabel *priceLabel = new QLabel("Price: " + QString::number(obj["minPrice"].toInt()) + " - " + QString::number(obj["maxPrice"].toInt()));
        QFont infoFont = typeLabel->font();
        infoFont.setPointSize(infoFont.pointSize() - 1);
        typeLabel->setFont(infoFont);
        descLabel->setFont(infoFont);
        locLabel->setFont(infoFont);
        priceLabel->setFont(infoFont);
        nameLabel->setStyleSheet("border: none; background: transparent;");
        typeLabel->setStyleSheet("border: none; background: transparent;");
        descLabel->setStyleSheet("border: none; background: transparent;");
        locLabel->setStyleSheet("border: none; background: transparent;");
        priceLabel->setStyleSheet("border: none; background: transparent;");
        QVBoxLayout *infoLayout = new QVBoxLayout();
        infoLayout->setContentsMargins(0, 0, 0, 0);
        infoLayout->setSpacing(2);
        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(typeLabel);
        infoLayout->addWidget(descLabel);
        infoLayout->addWidget(locLabel);
        infoLayout->addWidget(priceLabel);
        hbox->addLayout(infoLayout, 1);
        QPushButton *viewMenuBtn = new QPushButton("View Menu");
        hbox->addWidget(viewMenuBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
        card->setLayout(hbox);
        card->setFixedHeight(cardHeight);
        card->setMinimumHeight(cardHeight);
        card->setMaximumHeight(cardHeight);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        card->setStyleSheet("border: 1px solid #cccccc; border-radius: 0px; background: #fff;");
        vbox->addWidget(card);
        if (shownCount < cardCount - 1) {
            vbox->addSpacing(cardSpacing);
        }
        connect(viewMenuBtn, &QPushButton::clicked, [this, obj]() {
            int restaurantId = obj["id"].toInt();
            QString restaurantName = obj["name"].toString();
            NetworkManager* netManager = NetworkManager::getInstance();
            disconnect(netManager, &NetworkManager::menuReceived, this, nullptr);
            connect(netManager, &NetworkManager::menuReceived, this, [this, restaurantName, restaurantId](const QJsonArray &menu) {
                displayMenu(menu, restaurantName, restaurantId);
            });
            netManager->getRestaurantMenu(restaurantId);
        });
        ++shownCount;
    }
    vbox->setSizeConstraint(QLayout::SetMinAndMaxSize);
}

void customer::displayMenu(const QJsonArray &menu, const QString &restaurantName, int restaurantId) {
    // Create popup dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Menu for " + restaurantName);
    dialog->resize(600, 400);
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    // Add food type filter
    QComboBox* foodTypeCombo = new QComboBox(dialog);
    QSet<QString> foodTypes;
    for (const QJsonValue &val : menu) {
        QJsonObject item = val.toObject();
        foodTypes.insert(item["foodType"].toString());
    }
    foodTypeCombo->addItem("All");
    for (const QString &ft : foodTypes) foodTypeCombo->addItem(ft);
    layout->addWidget(foodTypeCombo);
    QTableWidget* table = new QTableWidget(dialog);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Type", "Name", "Details", "Price", "Order"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    auto updateTable = [=]() {
        table->setRowCount(0);
        QString selectedType = foodTypeCombo->currentText();
        int row = 0;
        for (int i = 0; i < menu.size(); ++i) {
            QJsonObject item = menu[i].toObject();
            if (selectedType != "All" && item["foodType"].toString() != selectedType)
                continue;
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(item["foodType"].toString()));
            table->setItem(row, 1, new QTableWidgetItem(item["foodName"].toString()));
            table->setItem(row, 2, new QTableWidgetItem(item["foodDetails"].toString()));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(item["price"].toInt())));
            QPushButton* orderBtn = new QPushButton("Order");
            table->setCellWidget(row, 4, orderBtn);
            QObject::connect(orderBtn, &QPushButton::clicked, this, [this, item, restaurantName, restaurantId, dialog]() {
                orderFood(restaurantId, item["id"].toInt(), restaurantName, item["foodName"].toString(), item["price"].toInt());
                // Optionally close dialog after ordering:
                // dialog->accept();
            });
            ++row;
        }
    };
    QObject::connect(foodTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), dialog, updateTable);
    layout->addWidget(table);
    dialog->setLayout(layout);
    updateTable();
    dialog->exec(); // Modal dialog
}

void customer::fetchAndDisplayRestaurants() {
    NetworkManager* netManager = NetworkManager::getInstance();
    disconnect(netManager, &NetworkManager::restaurantsReceived, this, nullptr);
    connect(netManager, &NetworkManager::restaurantsReceived, this, &customer::displayRestaurants);
    netManager->getRestaurants();
}

void customer::fetchAndDisplayMenu(int restaurantId, const QString &restaurantName) {
    NetworkManager* netManager = NetworkManager::getInstance();
    disconnect(netManager, &NetworkManager::menuReceived, this, nullptr);
    connect(netManager, &NetworkManager::menuReceived, this, [this, restaurantName, restaurantId](const QJsonArray &menu) {
        displayMenu(menu, restaurantName, restaurantId);
    });
    netManager->getRestaurantMenu(restaurantId);
}

void customer::orderFood(int restaurantId, int foodId, const QString &restaurantName, const QString &foodName, int price) {
    NetworkManager* netManager = NetworkManager::getInstance();
    QJsonArray items;
    QJsonObject itemObj;
    itemObj["foodId"] = foodId;
    itemObj["quantity"] = 1;
    itemObj["price"] = price;
    items.append(itemObj);
    connect(netManager, &NetworkManager::orderCreated, this, [this, foodName, restaurantName](const QString &msg) {
        QMessageBox::information(this, "Order Success", "Order placed for '" + foodName + "' at " + restaurantName + "\n" + msg);
    });
    connect(netManager, &NetworkManager::orderCreationFailed, this, [this](const QString &err) {
        QMessageBox::warning(this, "Order Failed", "Order failed: " + err);
    });
    netManager->createOrder(currentUserId, restaurantId, items, price);
}

void customer::fetchAndDisplayOrders() {
    if (currentUserId <= 0) return;
    NetworkManager* netManager = NetworkManager::getInstance();
    connect(netManager, &NetworkManager::ordersReceived, this, &customer::displayOrders, Qt::UniqueConnection);
    netManager->getOrders(QString::number(currentUserId), "customer");
}

void customer::displayOrders(const QJsonArray &orders) {
    QTableWidget *ordersTable = ui->ordersTableWidget;
    ordersTable->setRowCount(0);
    for (const QJsonValue &val : orders) {
        QJsonObject obj = val.toObject();
        int row = ordersTable->rowCount();
        ordersTable->insertRow(row);
        ordersTable->setItem(row, 0, new QTableWidgetItem(QString::number(obj["id"].toInt())));
        ordersTable->setItem(row, 1, new QTableWidgetItem(obj["createdAt"].toString()));
        ordersTable->setItem(row, 2, new QTableWidgetItem(obj["status"].toString()));
        ordersTable->setItem(row, 3, new QTableWidgetItem(QString::number(obj["totalAmount"].toInt())));
    }
}
