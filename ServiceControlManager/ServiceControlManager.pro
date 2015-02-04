#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T13:33:03
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scm
TEMPLATE = app

# Force prebuild version control info
#
# for creating version.h at first build
win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
unix:system([ -e ./version.h ] || touch ./version.h)
# for any build
versionTarget.target = version.h
versionTarget.depends = FORCE
win32 {
    versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
        nmake & \
        chdir $$PWD & \
        $$PWD/../GetGitProjectVersion.exe $$PWD/ServiceControlManager.pro
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/ServiceControlManager.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget

SOURCES += main.cpp\
        mainwindow.cpp \
    scanoptionswidget.cpp \
    servicetablemodel.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    DataSourcesStateWidget.cpp \
    ../lib/DataSource.cpp

HEADERS  += mainwindow.h \
    scanoptionswidget.h \
    servicetablemodel.h \
    ../include/UdpSocket.h \
    ../include/SocketIO.h \
    DataSourcesStateWidget.h \
    ../include/DataSource.h \
    version.h

FORMS    +=

TRANSLATIONS = ./translations/ServiceControlManager_ru.ts \
                ./translations/ServiceControlManager_uk.ts

RESOURCES += \
    ServiceControlManager.qrc

include(../qtsingleapplication/src/qtsingleapplication.pri)

*g++:QMAKE_CXXFLAGS += -std=c++11


# Visual Leak Detector
#
win32 {
        contains(QMAKE_TARGET.arch, x86_64) {
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
        } else {
                LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
                LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
        }

        INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
        INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}
