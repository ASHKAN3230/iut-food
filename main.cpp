#include "mainwindow.h"
#include "database_manager.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Initialize database
    DatabaseManager* dbManager = DatabaseManager::getInstance();
    if (!dbManager->initializeDatabase()) {
        QMessageBox::critical(nullptr, "Database Error", "Failed to initialize database. Application will exit.");
        return -1;
    }
    
    MainWindow w;
    w.show();
    return a.exec();
}
