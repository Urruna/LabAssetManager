#-------------------------------------------------
#
# Project created by QtCreator 2026-10-23T21:47:07
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = module_borrow
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        borrowpage.cpp

HEADERS += \
        borrowpage.h

FORMS += \
        borrowpage.ui

#使用公共数据库模块
INCLUDEPATH += $$PWD/../common
SOURCES += $$PWD/../common/databasemanager.cpp
HEADERS += $$PWD/../common/databasemanager.h

SOURCES += borrowdialog.cpp
HEADERS += borrowdialog.h
FORMS += borrowdialog.ui

SOURCES += returndialog.cpp
HEADERS += returndialog.h
FORMS += returndialog.ui

msvc:QMAKE_CXXFLAGS += /utf-8
