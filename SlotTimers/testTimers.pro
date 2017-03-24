#-------------------------------------------------
#
# Project created by QtCreator 2017-03-06T18:35:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += serialport

TARGET = testTimers
TEMPLATE = app


SOURCES += main.cpp \
    SlotTimers.cpp

HEADERS  += \
    SlotTimers.h

FORMS    += \
    SlotTimers.ui


