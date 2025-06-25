#include "clicklabel.h"
#include<QMouseEvent>

clicklabel::clicklabel(QWidget* parent): QLabel(parent)
{}

clicklabel::~clicklabel()
{}

void clicklabel::mousePressEvent(QMouseEvent* event)
{

    emit clicked();

    QLabel::mousePressEvent(event);

}
