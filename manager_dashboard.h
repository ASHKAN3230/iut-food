#ifndef MANAGER_DASHBOARD_H
#define MANAGER_DASHBOARD_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>

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
    void initRestaurantsTab();
    void initUsersTab();
    void initAuthApplicationsTab();
    void initOrdersAnalysisTab();
    QTableWidget *restaurantsTable = nullptr;
    QTableWidget *usersTable = nullptr;
    QTableWidget *authAppsTable = nullptr;
    QTableWidget *ordersTable = nullptr;
    QLabel *summaryLabel = nullptr;

private slots:
    void onRestaurantsReceived(const QJsonArray &restaurants);
    void onPendingAuthApplicationsReceived(const QJsonArray &apps);
    void onAllOrdersAndUsersReceived(const QJsonObject &data);
};

#endif // MANAGER_DASHBOARD_H 