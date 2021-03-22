QT      += core
QT      -= gui
QT      += network
QT	+= widgets
QT      += qml
QT      += xml

TARGET = BaseSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# c++20 support
#
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

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
    ../lib/Address16.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/Types.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Service.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/CommandLineParser.cpp \
    ../lib/WUtils.cpp \
    ../lib/XmlHelper.cpp \
    BaseServiceMain.cpp \
    ../lib/SoftwareInfo.cpp

HEADERS += \
	../lib/Address16.h \
    ../lib/SocketIO.h \
	../lib/SoftwareSettings.h \
	../lib/Types.h \
    ../lib/UdpSocket.h \
    ../lib/CircularLogger.h \
	../lib/FscDataFormat.h \
    ../lib/Service.h \
    ../lib/SimpleThread.h \
    ../lib/HostAddressPort.h \
    ../lib/CommandLineParser.h \
    ../lib/WUtils.h \
    ../lib/SoftwareInfo.h \
    ../lib/OrderedHash.h \
	../lib/XmlHelper.h \
	Stable.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf


include(../qtservice/src/qtservice.pri)

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
