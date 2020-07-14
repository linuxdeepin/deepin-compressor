#-------------------------------------------------
#
# Project created by QtCreator 2020-04-23T16:34:28
#
#-------------------------------------------------

QT  -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = libLogPlugin
TEMPLATE = lib

DEFINES += LIBLOGPLUGIN_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS LOG4QT_LIBRARY

LOG4QT_ROOT_PATH = $$PWD/log4qt

INCLUDEPATH += $$LOG4QT_ROOT_PATH

include($$LOG4QT_ROOT_PATH/log4qt.pri)

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


unix {
    target.path = /usr/lib
    INSTALLS += target
}
