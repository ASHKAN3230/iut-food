QT       += core gui

QT += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clicklabel.cpp \
    customer.cpp \
    forgot_password.cpp \
    main.cpp \
    mainwindow.cpp \
    menu_restaurant.cpp \
    order.cpp \
    rate.cpp \
    server.cpp \
    shopping_basket.cpp \
    sign_in.cpp

HEADERS += \
    clicklabel.h \
    customer.h \
    forgot_password.h \
    mainwindow.h \
    menu_restaurant.h \
    order.h \
    rate.h \
    server.h \
    shopping_basket.h \
    sign_in.h

FORMS += \
    customer.ui \
    forgot_password.ui \
    mainwindow.ui \
    menu_restaurant.ui \
    order.ui \
    rate.ui \
    shopping_basket.ui \
    sign_in.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    iut_image.qrc
