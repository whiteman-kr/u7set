QT += core
QT -= gui
QT += network
QT      += qml

CONFIG += c++11

TARGET = TuningSrv
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
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/TuningService.pro
        }
        else{
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/TuningService.pro
        }
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
        make; \
        cd $$PWD; \
        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/TuningService.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += \
    ../lib/BuildInfo.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Service.cpp \
    ../lib/SocketIO.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Tcp.cpp \
    ../lib/JsonSerializable.cpp \
    TuningService.cpp \
    ../lib/DataSource.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/Queue.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/OutputLog.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/Types.cpp \
    ../lib/Signal.cpp \
    ../AppDataService/AppSignalStateEx.cpp \
    ../lib/Crc.cpp \
    TuningMain.cpp \
    ../lib/WUtils.cpp \
    TuningDataStorage.cpp \
    ../lib/DataProtocols.cpp \
    ../u7/Builder/IssueLogger.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    TcpTuningServer.cpp \
    TcpTuningClient.cpp \
    TuningSource.cpp \
    ../u7/Builder/ModulesRawData.cpp \
    TuningSourceWorker.cpp \
    TuningMemory.cpp \
    TuningClientContext.cpp \
    ../lib/CommandLineParser.cpp

HEADERS += \
    ../lib/BuildInfo.h \
    ../lib/CfgServerLoader.h \
    ../lib/CircularLogger.h \
    ../lib/OrderedHash.h \
    ../lib/Service.h \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/SimpleThread.h \
    ../lib/WUtils.h \
    ../lib/TcpFileTransfer.h \
    ../lib/Tcp.h \
    TuningService.h \
    ../lib/DataSource.h \
    ../lib/XmlHelper.h \
    ../lib/ServiceSettings.h \
    ../lib/Queue.h \
    ../lib/DeviceHelper.h \
    ../lib/DeviceObject.h \
    ../lib/PropertyObject.h \
    ../lib/DbStruct.h \
    ../lib/OutputLog.h \
    ../lib/ProtoSerialization.h \
    ../lib/Types.h \
    ../lib/Signal.h \
    ../AppDataService/AppSignalStateEx.h \
    ../lib/Crc.h \
    TuningDataStorage.h \
    ../lib/Hash.h \
    ../u7/Builder/IssueLogger.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    TcpTuningServer.h \
    TcpTuningClient.h \
    TuningSource.h \
    ../u7/Builder/ModulesRawData.h \
    TuningSourceWorker.h \
    ../lib/DataProtocols.h \
    TuningMemory.h \
    TuningClientContext.h \
    ../lib/CommandLineParser.h

include(../qtservice/src/qtservice.pri)

win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

unix:QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

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

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto
