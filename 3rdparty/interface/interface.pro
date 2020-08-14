#-------------------------------------------------
#
# Project created by QtCreator 2020-08-12T14:42:38
#
#-------------------------------------------------

QT       += widgets core gui  KCodecs dtkgui dtkwidget

TARGET = interface
TEMPLATE = lib
PKGCONFIG += dtkwidget dtkgui
DEFINES += INTERFACE_LIBRARY

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
    analysepsdtool.cpp \
    archive_manager.cpp \
    archiveentry.cpp \
    archiveformat.cpp \
    archiveinterface.cpp \
    archivejob.cpp \
    archiverunnable.cpp \
    batchjobs.cpp \
    cliinterface.cpp \
    cliproperties.cpp \
    desktopfileparser.cpp \
    filewatcher.cpp \
    globalarchivemanager.cpp \
    jobs.cpp \
    kpluginfactory.cpp \
    kpluginloader.cpp \
    kpluginmetadata.cpp \
    kprocess.cpp \
    mimetypes.cpp \
    options.cpp \
    plugin.cpp \
    pluginmanager.cpp \
    queries.cpp \
    structs.cpp \
    TSMutex.cpp

HEADERS += \
    analysepsdtool.h \
    archive_manager.h \
    archiveentry.h \
    archiveformat.h \
    archiveinterface.h \
    archivejob_p.h \
    archivejob.h \
    archiverunnable.h \
    batchjobs.h \
    cliinterface.h \
    cliproperties.h \
    customdatainfo.h \
    desktopfileparser_p.h \
    filewatcher.h \
    globalarchivemanager_p.h \
    globalarchivemanager.h \
    jobs.h \
    kcoreaddons_export.h \
    kexportplugin.h \
    kpluginfactory_p.h \
    kpluginfactory.h \
    kpluginloader.h \
    kpluginmetadata.h \
    kprocess_p.h \
    kprocess.h \
    mimetypes.h \
    options.h \
    plugin.h \
    pluginmanager.h \
    queries.h \
    structs.h \
    tscommontypes.h \
    tsconstval.h \
    TSMutex.h \
    TSSingleton.h \
    tstypes.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

SUBDIRS += \
    interface.pro

DISTFILES += \
    CMakeLists.txt
