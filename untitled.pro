QT       += core gui sql network

QT += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

DEFINES += SRC_PATH=\\\"$$PWD/\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clicklabel.cpp \
    customer.cpp \
    database_manager.cpp \
    forgot_password.cpp \
    main.cpp \
    mainwindow.cpp \
    menu_restaurant.cpp \
    network_manager.cpp \
    order.cpp \
    rate.cpp \
    restaurant_auth.cpp \
    server.cpp \
    shopping_basket.cpp \
    sign_in.cpp \
    manager_dashboard.cpp \
    rate_dialog.cpp

HEADERS += \
    clicklabel.h \
    customer.h \
    database_manager.h \
    forgot_password.h \
    mainwindow.h \
    menu_restaurant.h \
    network_manager.h \
    order.h \
    rate.h \
    restaurant_auth.h \
    server.h \
    shopping_basket.h \
    sign_in.h \
    manager_dashboard.h \
    rate_dialog.h

FORMS += \
    customer.ui \
    forgot_password.ui \
    mainwindow.ui \
    menu_restaurant.ui \
    order.ui \
    rate.ui \
    restaurant_auth.ui \
    shopping_basket.ui \
    sign_in.ui \
    manager_dashboard.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    iut_image.qrc

