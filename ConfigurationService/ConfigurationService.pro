#-------------------------------------------------
#
# Project created by QtCreator 2014-11-18T12:51:14
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = CfgSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../bin_unix/release
}


## Force prebuild version control info
##
## for creating version.h at first build
#win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
#unix:system([ -e ./version.h ] || touch ./version.h)
## for any build
#versionTarget.target = version.h
#versionTarget.depends = FORCE
#win32 {
#        contains(QMAKE_TARGET.arch, x86_64){
#            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
#            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#            nmake & \
#            chdir $$PWD & \
#            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/ConfigurationService.pro
#        }
#        else{
#            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
#            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#            nmake & \
#            chdir $$PWD & \
#            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/ConfigurationService.pro
#        }
#}
#unix {
#    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
#        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
#        make; \
#        cd $$PWD; \
#        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/ConfigurationService.pro
#}
#PRE_TARGETDEPS += version.h
#QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += main.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SocketIO.cpp \
    ConfigurationService.cpp \
    ../lib/BaseService.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/JsonSerializable.cpp

HEADERS += \
    version.h \
    ../include/CfgServerLoader.h \
    ../include/Tcp.h \
    ../include/TcpFileTransfer.h \
    ../include/SimpleThread.h \
    ../include/BuildInfo.h \
    ../include/SocketIO.h \
    ConfigurationService.h \
    ../include/BaseService.h \
    ../include/UdpSocket.h \
    ../include/CircularLogger.h \
    ../include/JsonSerializable.h

include(../qtservice/src/qtservice.pri)

CONFIG(debug, debug|release): DEFINES += Q_DEBUG
