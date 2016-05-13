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
    ../u7/Builder/TuningDataStorage.cpp \
    ../lib/Signal.cpp \
    ../AppDataService/AppSignalState.cpp \
    ../lib/Crc.cpp \
    TuningMain.cpp

HEADERS += \
    ../include/BuildInfo.h \
    ../include/CfgServerLoader.h \
    ../include/CircularLogger.h \
    ../include/OrderedHash.h \
    ../include/Service.h \
    ../include/SocketIO.h \
    ../include/UdpSocket.h \
    ../include/SimpleThread.h \
    ../include/WUtils.h \
    ../include/TcpFileTransfer.h \
    ../include/Tcp.h \
    TuningSocket.h \
    TuningService.h \
    ../include/DataSource.h \
    ../include/XmlHelper.h \
    ../include/ServiceSettings.h \
    TuningDataSource.h \
    ../include/Queue.h \
    ../include/DeviceHelper.h \
    ../include/DeviceObject.h \
    ../include/PropertyObject.h \
    ../include/DbStruct.h \
    ../include/OutputLog.h \
    ../include/ProtoSerialization.h \
    ../include/Types.h \
    ../u7/Builder/TuningDataStorage.h \
    ../include/Signal.h \
    ../AppDataService/AppSignalState.h \
    ../include/Crc.h

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
