QT += network

TARGET = OnlineLib

TEMPLATE = lib
CONFIG += staticlib

include(../compiler.pri)
include(../warnings.pri)

INCLUDEPATH += $$PWD
INCLUDEPATH += ./../Protobuf

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

unix {
    target.path = /usr/lib
	INSTALLS += target
}

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

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
	MOC_DIR = debug/moc
	RCC_DIR = debug/rcc
	UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
	OBJECTS_DIR = release
	MOC_DIR = release/moc
	RCC_DIR = release/rcc
	UI_DIR = release/ui
}

HEADERS += \
	Stable.h \
	SoftwareInfo.h \
	Tcp.h \
    TcpClientStatistics.h \
	TcpFileTransfer.h \
	SocketIO.h \
	UdpSocket.h \
	CfgServerLoader.h \
	CircularLogger.h \
	SimpleAppSignalState.h \
	DataProtocols.h \

SOURCES += \
	SoftwareInfo.cpp \
	Tcp.cpp \
    TcpClientStatistics.cpp \
	TcpFileTransfer.cpp \
	SocketIO.cpp \
	UdpSocket.cpp \
	CfgServerLoader.cpp \
	CircularLogger.cpp \
	SimpleAppSignalState.cpp \
	DataProtocols.cpp \
