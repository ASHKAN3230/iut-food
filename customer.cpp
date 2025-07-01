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

customer::customer(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::customer)
{
    ui->setupUi(this);
    currentUsername = username;

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
