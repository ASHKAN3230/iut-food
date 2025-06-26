#ifndef SERVER_H
#define SERVER_H

#include <QObject>

class server : public QObject
{
    Q_OBJECT
public:
    explicit server(QObject *parent = nullptr);

};

#endif // SERVER_H
