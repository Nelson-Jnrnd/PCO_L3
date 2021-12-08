#-------------------------------------------------
#
# Project created by QtCreator 2015-02-12T16:05:55
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++17
config_threadsanitizer {
    QMAKE_CXXFLAGS += -fsanitize=thread
    LIBS += -ltsan
}

TARGET = PCO_Labo_3
TEMPLATE = app

LIBS += -lpcosynchro

SOURCES += main.cpp \
    machine.cpp

HEADERS  += \
    machine.h \
    machinemanager.h \
    machineinterface.h
