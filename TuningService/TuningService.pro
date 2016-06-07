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
    TuningSocket.cpp \
    TuningService.cpp \
    ../lib/DataSource.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/ServiceSettings.cpp \
    TuningDataSource.cpp \
    ../lib/Queue.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/PropertyObject.cpp \
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
    ../u7/Builder/IssueLogger.cpp

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
    TuningSocket.h \
    TuningService.h \
    ../lib/DataSource.h \
    ../lib/XmlHelper.h \
    ../lib/ServiceSettings.h \
    TuningDataSource.h \
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
    ../u7/Builder/IssueLogger.h

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
