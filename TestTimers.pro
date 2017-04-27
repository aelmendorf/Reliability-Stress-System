#-------------------------------------------------
#
# Project created by QtCreator 2017-04-04T15:40:09
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += serialport

TARGET = TestTimers
TEMPLATE = app


SOURCES += main.cpp\
        controller.cpp \
        twrap.cpp

HEADERS  += controller.h \
    twrap.h

FORMS    += controller.ui

OTHER_FILES += \
    ToDo.txt
