#include "menu_restaurant.h"
#include "ui_menu_restaurant.h"
#include "customer.h"
#include "shopping_basket.h"
#include<QFile>
#include<QLabel>
#include<QTextStream>
#include<QListWidgetItem>
#include<QSpinBox>
#include<QMessageBox>

menu_restaurant::menu_restaurant(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menu_restaurant)
{
    ui->setupUi(this);

    connect(this, &menu_restaurant::click_back_button, this, &menu_restaurant::send_message);

    connect(this, &menu_restaurant::receive_message, this, &menu_restaurant::on_back_button_clicked);

    connect(this, &menu_restaurant::click_shopping_basket_button, this, &menu_restaurant::send_message);

    connect(this, &menu_restaurant::receive_message, this, &menu_restaurant::on_shopping_basket_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

    open_menu_file();

}

menu_restaurant::~menu_restaurant()
{
    delete ui;
}

int menu_restaurant::index = 0;

void menu_restaurant::open_menu_file()
{

    QFile file_9("C:/Users/Asus/Desktop/customer-section/untitled/files/menu_list.txt");

    if(file_9.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_9);

        QString str;

        QString line;

        QString food_type;

        QString food_name;

        QString food_details;

        QString price;

        int counter = 0;

        while(!in.atEnd())
        {

            if(index == (counter/4))
            {

                line = in.readLine().trimmed();

                str = line + " | ";

                food_type = line;

                counter++;

                line = in.readLine().trimmed();

                str = str + line + " | ";

                food_name = line;

                counter++;

                line = in.readLine().trimmed();

                str = str + line + " | ";

                food_details = line;

                counter++;

                line = in.readLine().trimmed();

                str = str + line;

                price = line;

                counter++;

                QPair<QString,QString> food(food_details,price);

                menu_list[food_type][food_name] = food;

                QLabel *label = new QLabel(str);

                QSpinBox *spinbox = new QSpinBox();

                QListWidgetItem *li_1 = new QListWidgetItem(ui->bord_ListWidget);

                ui->bord_ListWidget->addItem(li_1);

                ui->bord_ListWidget->setItemWidget(li_1,label);

                QListWidgetItem *li_2 = new QListWidgetItem(ui->bord_ListWidget);

                ui->bord_ListWidget->addItem(li_2);

                ui->bord_ListWidget->setItemWidget(li_2,spinbox);

            }

        }

        file_9.close();

    }

}

void menu_restaurant::click_back_button()
{

    message = "back";

    emit click_server();

}

void menu_restaurant::on_back_button_clicked()
{

    customer *c = new customer();

    c->setAttribute(Qt::WA_DeleteOnClose);

    c->show();

    this->close();

}

void menu_restaurant::click_shopping_basket_button()
{

    message = "shop";

    emit click_server();

}

void menu_restaurant::on_shopping_basket_button_clicked()
{

    int j = 0;

    QString food_type;

    QString food_name;

    QString price;

    for(auto i = menu_list.begin(); i != menu_list.end(); ++i)
    {

        QListWidgetItem *item = ui->bord_ListWidget->item(j);

        if(j%2 == 0)
        {

                food_type = i.key();

                QMap<QString,QPair<QString,QString>> map = i.value();

                food_name = map.firstKey();

                QPair<QString,QString> food = map[food_name];

                price = food.second;

                shopping_basket::shop_basket[food_type][food_name] = food;

                ++j;

        }

        if(j%2 != 0)
        {

            QListWidgetItem *item = ui->bord_ListWidget->item(j);

            QWidget *widget = ui->bord_ListWidget->itemWidget(item);

            QSpinBox *spin = qobject_cast<QSpinBox*>(widget);

            if(spin)
            {

                shopping_basket::sum += (spin->value() * price.toInt());

            }

        }

        ++j;

    }

    shopping_basket *sb = new shopping_basket();

    sb->setAttribute(Qt::WA_DeleteOnClose);

    sb->show();

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

                emit click_back();

            }
            else if(message == "start shop")
            {

                message = "";

                emit click_shop();

            }

        }

    }

}

