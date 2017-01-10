#-------------------------------------------------
#
# Project created by QtCreator 2014-11-21T11:49:17
#
#-------------------------------------------------

QT  += core
QT  += network
QT  -= gui
QT  += widgets
QT  += qml
QT  += sql

TARGET = ArchSrv
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

# Force prebuild version control info
#
# for creating version.h at first build
win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
unix:system([ -e ./version.h ] || touch ./version.h)
# for any build
versionTarget.target = version.h
versionTarget.depends = FORCE
win32 {
	contains(QMAKE_TARGET.arch, x86_64){
	    versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
	    qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
	    nmake & \
	    chdir $$PWD & \
	    $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/ArchivingService.pro
	}
	else{
	    versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
	    qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
	    nmake & \
	    chdir $$PWD & \
	    $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/ArchivingService.pro
	}
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
	qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
	make; \
	cd $$PWD; \
	$$PWD/../bin_unix/GetGitProjectVersion $$PWD/ArchivingService.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += \
    ../lib/HostAddressPort.cpp \
    ArchivingService.cpp \
    ../lib/Queue.cpp \
    ../lib/Service.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/SocketIO.cpp \
    ../lib/Tcp.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/CfgServerLoader.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/UdpSocket.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/DeviceObject.cpp \
    ../u7/Builder/IssueLogger.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/OutputLog.cpp \
    ../lib/Types.cpp \
    ../u7/Builder/ModulesRawData.cpp \
    ../lib/CommandLineParser.cpp \
    ArchServiceMain.cpp

HEADERS += \
    version.h \
    ../lib/HostAddressPort.h \
    ArchivingService.h \
    Stable.h \
    ../lib/Queue.h \
    ../lib/Service.h \
    ../lib/ServiceSettings.h \
    ../lib/Address16.h \
    ../lib/OrderedHash.h \
    ../lib/SimpleThread.h \
    ../lib/SocketIO.h \
    ../lib/Tcp.h \
    ../lib/XmlHelper.h \
    ../lib/CfgServerLoader.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/UdpSocket.h \
    ../lib/BuildInfo.h \
    ../lib/CircularLogger.h \
    ../lib/DeviceHelper.h \
    ../lib/JsonSerializable.h \
    ../lib/TcpFileTransfer.h \
    ../lib/DeviceObject.h \
    ../u7/Builder/IssueLogger.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/OutputLog.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../u7/Builder/ModulesRawData.h \
    ../lib/CommandLineParser.h

include(../qtservice/src/qtservice.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

win32 {
        LIBS += -L$$DESTDIR -lprotobuf

        INCLUDEPATH += ./../Protobuf
}
unix {
        LIBS += -lprotobuf
}

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto


