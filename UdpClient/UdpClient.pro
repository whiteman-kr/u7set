#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T16:12:02
#
#-------------------------------------------------

QT       += core gui

QT       += network qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpClient
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
            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/UdpClient.pro
        }
        else{
            versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
            nmake & \
            chdir $$PWD & \
            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/UdpClient.pro
        }
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
        MainWindow.cpp \
        ClientMain.cpp \
        ../lib/UdpSocket.cpp \
	../lib/Service.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/DataSource.cpp \
    FscDataSource.cpp \
    ../lib/ProtoUdp.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/Tcp.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/WUtils.cpp \
    ../lib/Crc.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/HostAddressPort.cpp \
    ../TuningService/TcpTuningClient.cpp \
    ../TuningService/TuningSource.cpp \
    ../lib/PropertyObject.cpp \
    ../lib/Signal.cpp \
    ../TuningService/TuningDataStorage.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/Types.cpp \
    ../lib/OutputLog.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/CommandLineParser.cpp


HEADERS  += MainWindow.h \
        ../lib/SocketIO.h \
        ../lib/UdpSocket.h \
	../lib/Service.h \
    ../lib/CircularLogger.h \
    ../lib/DataSource.h \
	FscDataSource.h \
	../lib/FscDataFormat.h \
    version.h \
    ../lib/ProtoUdp.h \
    ../lib/SimpleThread.h \
    ../lib/Tcp.h \
    ../lib/JsonSerializable.h \
    ../lib/XmlHelper.h \
    ../lib/Queue.h \
    ../lib/DataProtocols.h \
    ../lib/WUtils.h \
    ../lib/Crc.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/HostAddressPort.h \
    ../TuningService/TcpTuningClient.h \
    ../TuningService/TuningSource.h \
    ../lib/PropertyObject.h \
    ../lib/Signal.h \
    ../TuningService/TuningDataStorage.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/Types.h \
    ../lib/OutputLog.h \
    ../lib/ProtoSerialization.h \
    ../lib/CommandLineParser.h

include(../qtservice/src/qtservice.pri)

FORMS    += MainWindow.ui

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
