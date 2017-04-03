QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml

TARGET = DiagDataSrv
CONFIG += console
CONFIG -= app_bundle

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
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/DiagDataService.pro
        }
        else{
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/DiagDataService.pro
        }
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/DiagDataService.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += \
	../lib/UdpSocket.cpp \
	../lib/Service.cpp \
	../lib/SocketIO.cpp \
	../lib/CircularLogger.cpp \
    ../lib/DataSource.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Signal.cpp \
    ../lib/Types.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/OutputLog.cpp \
    ../lib/XmlHelper.cpp \
    DiagDataService.cpp \
    ../lib/DataChannel.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/WUtils.cpp \
    ../u7/Builder/IssueLogger.cpp \
    ../lib/Crc.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../u7/Builder/ModulesRawData.cpp \
    ../lib/CommandLineParser.cpp \
    DiagDataServiceMain.cpp

HEADERS += \
	Stable.h \
	../lib/SocketIO.h \
	../lib/UdpSocket.h \
	../lib/Service.h \
	../lib/CircularLogger.h \
    ../lib/DataSource.h \
    version.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/Signal.h \
    ../lib/CUtils.h \
    ../lib/PropertyObject.h \
    ../lib/Types.h \
    ../lib/JsonSerializable.h \
    ../lib/CfgServerLoader.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
    ../lib/ServiceSettings.h \
    ../lib/DeviceHelper.h \
    ../lib/OutputLog.h \
    ../lib/XmlHelper.h \
    ../lib/DataProtocols.h \
    DiagDataService.h \
    ../lib/DataChannel.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../u7/Builder/IssueLogger.h \
    ../lib/Crc.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
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
