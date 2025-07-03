#ifndef MANAGER_DASHBOARD_H
#define MANAGER_DASHBOARD_H

#include <QWidget>

namespace Ui {
class manager_dashboard;
}

class manager_dashboard : public QWidget
{
    Q_OBJECT

public:
    explicit manager_dashboard(const QString &username, QWidget *parent = nullptr);
    ~manager_dashboard();

private:
    Ui::manager_dashboard *ui;
    QString currentManagerUsername;
};

#endif // MANAGER_DASHBOARD_H 