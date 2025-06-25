#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include<QLineEdit>
#include "clicklabel.h"
#include<string>
#include<QTcpSocket>
#include<QTcpServer>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    void run();

    void click_login_button();

    std::string message;

    void receive_message();

    QTcpSocket socket;

    ~MainWindow();

signals:

    void click();

    void click_server();

public slots:

    void open_signin_window();

    void open_forgot_window();

    void on_login_button_clicked();

    void send_message();

private:
    Ui::MainWindow *ui;

    clicklabel *signin_label;

    clicklabel *forgot_password_label;

};
#endif // MAINWINDOW_H

