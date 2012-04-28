#-------------------------------------------------
#
# Project created by QtCreator 2012-04-12T21:54:56
#
#-------------------------------------------------

QT       += core gui

TARGET = debugger_gui
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../../debugger/debugger.cpp \
    ../../dcpu16/dcpu16.cpp \
    memory_view.cpp \
    gui_utils.cpp

HEADERS  += mainwindow.h \
    ../../debugger/debugger.h \
    ../../dcpu16/dcpu16.h \
    memory_view.h \
    gui_utils.h

FORMS    += mainwindow.ui
