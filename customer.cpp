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

customer::customer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::customer)
{
    ui->setupUi(this);

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

        while(!in.atEnd())
        {

            clicklabel *cl = new clicklabel();

            cl->setStyleSheet(" font: 9pt Segoe UI;color: rgb(250, 144, 169);background-color: rgba(255,255,255,0);");

            key_type = in.readLine().trimmed();

            line = key_type;

            line = line + " ";

            cl->setText(line);

            key_name = in.readLine().trimmed();

            line = line + key_name;

            line = line + " ";

            cl->setText(line);

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

            QListWidgetItem *li = new QListWidgetItem(ui->bord_ListWidget);

            ui->bord_ListWidget->setItemWidget(li,cl);

            connect(cl, &clicklabel::clicked, [this,index](){this->open_next_window(index);});

            line = in.readLine();

            index++;

        }

        file_8.close();

    }

}

void customer::open_next_window(int new_index)
{

    menu_restaurant::index = new_index;

    menu_restaurant *mr = new menu_restaurant();

    mr->setAttribute(Qt::WA_DeleteOnClose);

    mr->show();

    this->close();

}

void customer::click_search_button()
{

    message = "search";

    emit click_server();

}

void customer::on_search_button_clicked()
{

    QString line;

    int index_1 = 0;

    QString minimum_price = ui->minimum_price_lineedit->text();

    int min_price = minimum_price.toInt();

    QString maximum_price = ui->maximum_price_lineedit->text();

    int max_price = maximum_price.toInt();

    QString position = ui->position_lineedit->text();

    ui->bord_ListWidget->clear();

    //searh about price
    for(auto i = restaurant_list.begin(); i != restaurant_list.end(); ++i)
    {

        QMap<QString,QPair<QVector<QString>, QPair<int,int>>> &restaurant_name = i.value();

        for(auto j = restaurant_name.begin(); j != restaurant_name.end(); ++j)
        {

            QPair<QVector<QString>, QPair<int,int>> restaurant_position = j.value();

            QPair<int,int> range = restaurant_position.second;

            if(min_price <= range.first && range.second <= max_price)
            {

                clicklabel *cl = new clicklabel();

                QListWidgetItem *li = new QListWidgetItem(ui->bord_ListWidget);

                line = i.key() + " " + j.key();

                cl->setText(line);

                cl->setStyleSheet(" font: 9pt Segoe UI;color: rgb(250, 144, 169);background-color: rgba(255,255,255,0);");

                ui->bord_ListWidget->setItemWidget(li,cl);

                connect(cl, &clicklabel::clicked, [this,index_1](){this->open_next_window(index_1);});

                index_1++;

            }

        }

    }
    //search about position
    QStringList list = position.split("-");

    QVector<QString> position_section = list.toVector();

    index_1 = 0;

    int index_2 = 0;

    bool state = false;

    if((min_price == 0 && max_price == 0) || (min_price != 0 && max_price != 0 && position != ""))
    {

        ui->bord_ListWidget->clear();

    }

    for(auto i = restaurant_list.begin(); i != restaurant_list.end(); ++i)
    {

        QMap<QString,QPair<QVector<QString>, QPair<int,int>>> &restaurant_name = i.value();

        for(auto j = restaurant_name.begin(); j != restaurant_name.end(); ++j)
        {

            QPair<QVector<QString>, QPair<int,int>> restaurant_position = j.value();

            QPair<int,int> range = restaurant_position.second;

            for(QString &s: position_section)
            {

                if(s != restaurant_position.first[index_2])
                {

                    state = false;

                    break;

                }
                else
                {

                    if(min_price == 0 && max_price == 0)
                    {

                        state = true;

                        index_2++;

                    }
                    else if(min_price != 0 && max_price != 0)
                    {

                        if(min_price <= range.first && range.second <= max_price)
                        {

                            state = true;

                            index_2++;

                        }

                    }

                }

            }

            if(state == true)
            {

                clicklabel *cl = new clicklabel();

                QListWidgetItem *li = new QListWidgetItem(ui->bord_ListWidget);

                line = i.key() + " " + j.key();

                cl->setText(line);

                cl->setStyleSheet(" font: 9pt Segoe UI;color: rgb(250, 144, 169);background-color: rgba(255,255,255,0);");

                ui->bord_ListWidget->setItemWidget(li,cl);

                connect(cl, &clicklabel::clicked, [this,index_1](){this->open_next_window(index_1);});

                index_1++;

            }

            index_2 = 0;

            state = false;

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
