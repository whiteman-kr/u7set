#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T16:12:02
#
#-------------------------------------------------

QT       += core gui

QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpClient
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
        $$PWD/../GetGitProjectVersion.exe $$PWD/UdpClient.pro
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/UdpClient.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += \
        mainwindow.cpp \
        clientmain.cpp \
        ../lib/UdpSocket.cpp \
    ../lib/BaseService.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/DataSource.cpp \
    FscDataSource.cpp


HEADERS  += mainwindow.h \
        ../include/SocketIO.h \
        ../include/UdpSocket.h \
    ../include/BaseService.h \
    ../include/CircularLogger.h \
    ../include/DataSource.h \
	FscDataSource.h \
	../include/FscDataFormat.h \
    version.h

include(../qtservice/src/qtservice.pri)

FORMS    += mainwindow.ui

unix:QMAKE_CXXFLAGS += -std=c++11


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

