#-------------------------------------------------
#
# Project created by QtCreator 2019-05-17T15:25:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kallistoscope
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

CONFIG += c++11

SOURCES += \
        ui/computeresults.cpp \
        ui/indexcreation.cpp \
        ui/installation.cpp \
        ui/headerbar.cpp \
        ui/indexfile.cpp \
        logic/calculation.cpp \
        logic/database.cpp \
        logic/filedownloads.cpp \
        ui/main.cpp \
        ui/mainwindow.cpp \
        ui/newproject.cpp \
        ui/openproject.cpp \
        logic/project.cpp \
        ui/removeprojects.cpp \
        ui/viewprojecthistory.cpp

HEADERS += \
        ui/computeresults.h \
        ui/indexcreation.h \
        ui/installation.h \
        ui/headerbar.h \
        ui/indexfile.h \
        logic/calculation.h \
        logic/database.h \
        logic/filedownloads.h \
        logic/kseq.h \
        ui/mainwindow.h \
        ui/newproject.h \
        ui/openproject.h \
        logic/project.h \
        ui/removeprojects.h \
        ui/viewprojecthistory.h

FORMS += \
        ui/computeresults.ui \
        ui/installation.ui \
        ui/headerbar.ui \
        ui/indexfile.ui \
        ui/indexcalculation.ui \
        ui/mainwindow.ui \
        ui/newproject.ui \
        ui/openproject.ui \
        ui/removeprojects.ui \
        ui/viewprojecthistory.ui

QT += sql
QT += network

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32: LIBS += -L$$PWD/zlib-1.2.11 -lzlib
#else:linux-g++: LIBS +=-Wl,-Bstatic,-L$$PWD/zlib-1.2.11 -lz -Wl,-Bdynamic
#else:mac: LIBS += -L$$PWD/zlib-1.2.11/ -lz

LIBS += -lz

#INCLUDEPATH += $$PWD/zlib-1.2.11
#DEPENDPATH += $$PWD/zlib-1.2.11

#win32-g++: PRE_TARGETDEPS += $$PWD/zlib-1.2.11/libz.a
#else:!win32-g++: PRE_TARGETDEPS += $$PWD/zlib-1.2.11/zlib.dll
#else:unix: PRE_TARGETDEPS += $$PWD/zlib-1.2.11/libz.a

#QMAKE_LFLAGS += -static-$$PWD/zlib-1.2.11/libz.a
linux-g++ {
    QMAKE_LFLAGS += -no-pie
}

DISTFILES += \
    Snakefile.template
