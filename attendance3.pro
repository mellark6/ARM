QT       += core gui
QT += positioning core
QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QT += sql widgets
QT += widgets
QT += sql
QT += positioning
QT += network
QT += core gui sql network

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    studentwindow.cpp \
    tutorwindow.cpp

HEADERS += \
    mainwindow.h \
    studentwindow.h \
    studentwindow.h \
    tutorwindow.h

FORMS += \
    mainwindow.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
