#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include<Qlabel>
#include<QWidget>
#include<Qt>
#include<string>

class clicklabel: public QLabel
{

    Q_OBJECT

public:
    clicklabel(QWidget* parent = nullptr);

    std::string message;

    ~clicklabel();

signals:

    void clicked();

protected:

    void mousePressEvent(QMouseEvent* event)override;

};

#endif // CLICKLABEL_H
