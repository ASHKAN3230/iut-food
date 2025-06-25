#include "order.h"
#include "ui_order.h"
#include "shopping_basket.h"
#include "rate.h"//پیش فرض
#include<QMessageBox>
#include<QLabel>

order::order(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::order)
{
    ui->setupUi(this);

    connect(this, &order::click_back_button, this, &order::send_message);

    connect(this, &order::receive_message, this, &order::on_back_button_clicked);

    connect(this, &order::click_order_tracking, this, &order::send_message);

    connect(this, &order::receive_message, this, &order::on_order_tracking_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

    show_orders();

}

order::~order()
{
    delete ui;
}

QMap<QString,QMap<QString,QPair<QString,QString>>> order::order_history;

int order::price;

void order::show_orders()
{

    QString food_type;

    QString food_name;

    QString price_order;

    for(auto i = order_history.begin(); i != order_history.end(); ++i)
    {

        food_type = i.key();

        QMap<QString,QPair<QString,QString>> shop = i.value();

        food_name = shop.firstKey();

        QPair<QString,QString> str = shop[food_name];

        price_order = str.second;

        QLabel *label = new QLabel();

        label->setText(food_type + " | " + food_name + " | " + price_order);

        QListWidgetItem *li = new QListWidgetItem(ui->bord_ListWidget);

        ui->bord_ListWidget->setItemWidget(li,label);

    }

    QLabel *label = new QLabel();

    price_order = QString::number(price);

    label->setText("total price: " + price_order);

    QListWidgetItem *li = new QListWidgetItem(ui->bord_ListWidget);

    ui->bord_ListWidget->setItemWidget(li,label);

}

void order::click_back_button()
{

    message = "back";

    emit click_server();

}

void order::click_order_tracking()
{

    message = "tracking";

    emit click_server();

}

void order::on_back_button_clicked()
{

    shopping_basket *sb = new shopping_basket();

    sb->setAttribute(Qt::WA_DeleteOnClose);

    sb->show();

    this->close();

}

void order::receive_message()
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
            else if(message == "start tracking")
            {

                message = "";

                emit click_tracking();

            }

        }

    }

}

void order::send_message()
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

void order::on_order_tracking_button_clicked()
{
    //پیش فرض
    rate *r = new rate();

    r->setAttribute(Qt::WA_DeleteOnClose);

    r->show();

    this->close();

}

