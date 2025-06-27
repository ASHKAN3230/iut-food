#include "rate.h"
#include "ui_rate.h"
#include "order.h"//پیش فرض
#include<QMessageBox>
#include<QFile>
#include<QString>
#include<QLabel>
#include<QlineEdit>
#include<QSpinBox>
#include<QRadioButton>
#include<QTextStream>

rate::rate(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::rate)
{
    ui->setupUi(this);
    currentUsername = username;

    set_items_for_rate();

    connect(this, &rate::click_back_button, this, &rate::send_message);

    connect(this, &rate::receive_message, this, &rate::on_back_button_clicked);

    connect(this, &rate::click_save_button, this, &rate::send_message);

    connect(this, &rate::receive_message, this, &rate::on_save_button_clicked);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {

        receive_message();

    }

    show_rates();

}

rate::~rate()
{
    delete ui;
}

int rate::index = 0;

int rate::order_count = 0;

void rate::click_back_button()
{

    message = "back";

    emit click_server();

}

void rate::click_save_button()
{

    message = "save";

    emit click_server();

}

void rate::show_rates()
{

    QString state;

    QString rate_number;

    QString comment;

    QFile file_10("files/rate_list.txt");

    if(file_10.open(QIODevice::ReadOnly | QIODevice::Text))
    {

        QTextStream in(&file_10);

        while(!in.atEnd())
        {

            state = in.readLine();

            rate_number = in.readLine();

            comment = in.readLine();

            QPair<QString,QString> data;

            data.first = rate_number;

            data.second = comment;

            QPair<QString,QPair<QString,QString>> str;

            str.first = state;

            str.second = data;

            rate_information[index] = str;

            ++index;

            QLabel *label_1 = new QLabel();

            QLabel *label_2 = new QLabel();

            QLabel *label_3 = new QLabel();

            label_1->setText(state);

            label_2->setText(rate_number);

            label_3->setText(comment);

            QListWidgetItem *li_1 = new QListWidgetItem(ui->bord_ListWidget);

            QListWidgetItem *li_2 = new QListWidgetItem(ui->bord_ListWidget);

            QListWidgetItem *li_3 = new QListWidgetItem(ui->bord_ListWidget);

            ui->bord_ListWidget->setItemWidget(li_1,label_1);

            ui->bord_ListWidget->setItemWidget(li_2,label_2);

            ui->bord_ListWidget->setItemWidget(li_3,label_3);

        }

        file_10.close();

    }

    for(int i = 0; i < order_count; ++i)
    {

        QRadioButton *rb = new QRadioButton();

        rb->setText("received");

        QSpinBox *sb = new QSpinBox();

        QLineEdit *le = new QLineEdit();

        QListWidgetItem *li_1 = new QListWidgetItem(ui->bord_ListWidget);

        QListWidgetItem *li_2 = new QListWidgetItem(ui->bord_ListWidget);

        QListWidgetItem *li_3 = new QListWidgetItem(ui->bord_ListWidget);

        ui->bord_ListWidget->setItemWidget(li_1,rb);

        ui->bord_ListWidget->setItemWidget(li_2,sb);

        ui->bord_ListWidget->setItemWidget(li_3,le);

    }

}

void rate::on_save_button_clicked()
{

    QFile file_11("files/rate_list.txt");

    if(file_11.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
    {

        QTextStream out(&file_11);

        for(int i = (index*3); i < ((order_count+index)*3); i+=3)
        {

            QListWidgetItem *item_1 = ui->bord_ListWidget->item(i);

            QListWidgetItem *item_2 = ui->bord_ListWidget->item(i+1);

            QListWidgetItem *item_3 = ui->bord_ListWidget->item(i+2);

            QWidget *widget_1 = ui->bord_ListWidget->itemWidget(item_1);

            QWidget *widget_2 = ui->bord_ListWidget->itemWidget(item_2);

            QWidget *widget_3 = ui->bord_ListWidget->itemWidget(item_3);

            QRadioButton *radio = qobject_cast<QRadioButton*>(widget_1);

            QSpinBox *spinbox = qobject_cast<QSpinBox*>(widget_2);

            QLineEdit *lineedit = qobject_cast<QLineEdit*>(widget_3);

            if(radio->isChecked())
            {

                out << "true";

                out << "\n";

                out << QString::number(spinbox->value());

                out << "\n";

                out << lineedit->text();

                out << "\n";

            }
            else
            {

                out << "false";

                out << "\n";

                out << QString::number(spinbox->value());

                out << "\n";

                out << lineedit->text();

                out << "\n";

            }

        }

        file_11.close();

    }

}

void rate::on_back_button_clicked()
{

    order *Order = new order(currentUsername);

    Order->setAttribute(Qt::WA_DeleteOnClose);

    Order->show();

    this->close();

}

void rate::send_message()
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

void rate::receive_message()
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
            else if(message == "start save")
            {

                message = "";

                emit click_save();

            }

        }

    }

}

void rate::set_items_for_rate()
{
    // TODO: Implement this function
}
