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
#include<QInputDialog>

menu_restaurant::menu_restaurant(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::menu_restaurant)
    , selectedItemIndex(-1)
{
    ui->setupUi(this);

    connect(this, &menu_restaurant::click_back_button, this, &menu_restaurant::send_message);

    connect(this, &menu_restaurant::receive_message, this, &menu_restaurant::on_back_button_clicked);

    connect(this, &menu_restaurant::click_shopping_basket_button, this, &menu_restaurant::send_message);

    connect(this, &menu_restaurant::receive_message, this, &menu_restaurant::on_shopping_basket_button_clicked);

    // Connect new food management buttons
    connect(ui->addFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_addFoodButton_clicked);
    connect(ui->editFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_editFoodButton_clicked);
    connect(ui->deleteFoodButton, &QPushButton::clicked, this, &menu_restaurant::on_deleteFoodButton_clicked);
    connect(ui->clearFormButton, &QPushButton::clicked, this, &menu_restaurant::on_clearFormButton_clicked);
    connect(ui->bord_ListWidget, &QListWidget::itemClicked, this, &menu_restaurant::on_menuItem_selected);

    socket.connectToHost("127.0.0.1",6006);

    if(socket.waitForConnected(1000))
    {
        receive_message();
    }

    open_menu_file();
    refresh_menu_display();
}

menu_restaurant::~menu_restaurant()
{
    delete ui;
}

int menu_restaurant::index = 0;

void menu_restaurant::open_menu_file()
{
    QFile file_9("files/menu_list.txt");

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
            }
            else
            {
                // Skip lines for other restaurants
                for(int i = 0; i < 4 && !in.atEnd(); i++)
                {
                    in.readLine();
                    counter++;
                }
            }
        }
        file_9.close();
    }
}

void menu_restaurant::save_menu_to_file()
{
    QFile file("files/menu_list.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        
        // Write all restaurants' menus
        for(auto restaurantIt = menu_list.begin(); restaurantIt != menu_list.end(); ++restaurantIt)
        {
            QString foodType = restaurantIt.key();
            QMap<QString, QPair<QString, QString>> foods = restaurantIt.value();
            
            for(auto foodIt = foods.begin(); foodIt != foods.end(); ++foodIt)
            {
                QString foodName = foodIt.key();
                QPair<QString, QString> food = foodIt.value();
                
                out << foodType << "\n";
                out << foodName << "\n";
                out << food.first << "\n";  // details
                out << food.second << "\n"; // price
            }
        }
        file.close();
    }
}

void menu_restaurant::refresh_menu_display()
{
    ui->bord_ListWidget->clear();
    
    int itemIndex = 0;
    for(auto restaurantIt = menu_list.begin(); restaurantIt != menu_list.end(); ++restaurantIt)
    {
        QString foodType = restaurantIt.key();
        QMap<QString, QPair<QString, QString>> foods = restaurantIt.value();
        
        for(auto foodIt = foods.begin(); foodIt != foods.end(); ++foodIt)
        {
            QString foodName = foodIt.key();
            QPair<QString, QString> food = foodIt.value();
            
            QString displayText = foodType + " | " + foodName + " | " + food.first + " | " + food.second;
            
            QLabel *label = new QLabel(displayText);
            QSpinBox *spinbox = new QSpinBox();
            spinbox->setMinimum(0);
            spinbox->setMaximum(99);
            
            QListWidgetItem *li_1 = new QListWidgetItem(ui->bord_ListWidget);
            ui->bord_ListWidget->addItem(li_1);
            ui->bord_ListWidget->setItemWidget(li_1, label);
            
            QListWidgetItem *li_2 = new QListWidgetItem(ui->bord_ListWidget);
            ui->bord_ListWidget->addItem(li_2);
            ui->bord_ListWidget->setItemWidget(li_2, spinbox);
            
            itemIndex += 2;
        }
    }
}

void menu_restaurant::clear_form()
{
    ui->foodTypeEdit->clear();
    ui->foodNameEdit->clear();
    ui->foodDetailsEdit->clear();
    ui->priceSpinBox->setValue(0);
    selectedItemIndex = -1;
    selectedFoodType.clear();
    selectedFoodName.clear();
}

void menu_restaurant::on_addFoodButton_clicked()
{
    QString foodType = ui->foodTypeEdit->text().trimmed();
    QString foodName = ui->foodNameEdit->text().trimmed();
    QString foodDetails = ui->foodDetailsEdit->text().trimmed();
    QString price = QString::number(ui->priceSpinBox->value());
    
    if(foodType.isEmpty() || foodName.isEmpty() || foodDetails.isEmpty() || price == "0")
    {
        QMessageBox::warning(this, "Input Error", "Please fill all fields with valid values!");
        return;
    }
    
    // Check if food already exists
    if(menu_list[foodType].contains(foodName))
    {
        QMessageBox::warning(this, "Duplicate Food", "A food with this name already exists in this category!");
        return;
    }
    
    QPair<QString, QString> food(foodDetails, price);
    menu_list[foodType][foodName] = food;
    
    save_menu_to_file();
    refresh_menu_display();
    clear_form();
    
    QMessageBox::information(this, "Success", "Food item added successfully!");
}

void menu_restaurant::on_editFoodButton_clicked()
{
    if(selectedItemIndex == -1)
    {
        QMessageBox::warning(this, "Selection Error", "Please select a food item to edit!");
        return;
    }
    
    QString newFoodType = ui->foodTypeEdit->text().trimmed();
    QString newFoodName = ui->foodNameEdit->text().trimmed();
    QString newFoodDetails = ui->foodDetailsEdit->text().trimmed();
    QString newPrice = QString::number(ui->priceSpinBox->value());
    
    if(newFoodType.isEmpty() || newFoodName.isEmpty() || newFoodDetails.isEmpty() || newPrice == "0")
    {
        QMessageBox::warning(this, "Input Error", "Please fill all fields with valid values!");
        return;
    }
    
    // Remove old entry
    if(menu_list.contains(selectedFoodType) && menu_list[selectedFoodType].contains(selectedFoodName))
    {
        menu_list[selectedFoodType].remove(selectedFoodName);
        
        // Remove empty food type
        if(menu_list[selectedFoodType].isEmpty())
        {
            menu_list.remove(selectedFoodType);
        }
    }
    
    // Add new entry
    QPair<QString, QString> food(newFoodDetails, newPrice);
    menu_list[newFoodType][newFoodName] = food;
    
    save_menu_to_file();
    refresh_menu_display();
    clear_form();
    
    QMessageBox::information(this, "Success", "Food item updated successfully!");
}

void menu_restaurant::on_deleteFoodButton_clicked()
{
    if(selectedItemIndex == -1)
    {
        QMessageBox::warning(this, "Selection Error", "Please select a food item to delete!");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete", 
        "Are you sure you want to delete this food item?", 
        QMessageBox::Yes | QMessageBox::No);
    
    if(reply == QMessageBox::Yes)
    {
        if(menu_list.contains(selectedFoodType) && menu_list[selectedFoodType].contains(selectedFoodName))
        {
            menu_list[selectedFoodType].remove(selectedFoodName);
            
            // Remove empty food type
            if(menu_list[selectedFoodType].isEmpty())
            {
                menu_list.remove(selectedFoodType);
            }
        }
        
        save_menu_to_file();
        refresh_menu_display();
        clear_form();
        
        QMessageBox::information(this, "Success", "Food item deleted successfully!");
    }
}

void menu_restaurant::on_clearFormButton_clicked()
{
    clear_form();
}

void menu_restaurant::on_menuItem_selected()
{
    QListWidgetItem *item = ui->bord_ListWidget->currentItem();
    if(!item) return;
    
    int row = ui->bord_ListWidget->row(item);
    if(row % 2 == 0) // Only handle label items (even rows)
    {
        QWidget *widget = ui->bord_ListWidget->itemWidget(item);
        QLabel *label = qobject_cast<QLabel*>(widget);
        if(label)
        {
            QString text = label->text();
            QStringList parts = text.split(" | ");
            
            if(parts.size() >= 4)
            {
                selectedFoodType = parts[0];
                selectedFoodName = parts[1];
                QString foodDetails = parts[2];
                QString price = parts[3];
                
                ui->foodTypeEdit->setText(selectedFoodType);
                ui->foodNameEdit->setText(selectedFoodName);
                ui->foodDetailsEdit->setText(foodDetails);
                ui->priceSpinBox->setValue(price.toInt());
                
                selectedItemIndex = row;
            }
        }
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

