#include "manager_dashboard.h"
#include "ui_manager_dashboard.h"

manager_dashboard::manager_dashboard(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::manager_dashboard)
{
    ui->setupUi(this);
    currentManagerUsername = username;
}

manager_dashboard::~manager_dashboard()
{
    delete ui;
} 
