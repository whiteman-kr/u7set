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

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet
win32:QMAKE_CXXFLAGS += /analyze

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
    ../lib/MemLeaksDetection.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/Types.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Service.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/CommandLineParser.cpp \
    ../lib/WUtils.cpp \
    ../lib/XmlHelper.cpp \
    BaseServiceMain.cpp \
    ../lib/SoftwareInfo.cpp

HEADERS += \
	../lib/Address16.h \
	../lib/MemLeaksDetection.h \
    ../lib/SocketIO.h \
	../lib/SoftwareSettings.h \
	../lib/Types.h \
    ../lib/UdpSocket.h \
    ../lib/CircularLogger.h \
	../lib/FscDataFormat.h \
    ../lib/Service.h \
    ../lib/SimpleThread.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/CommandLineParser.h \
    ../lib/WUtils.h \
    ../lib/SoftwareInfo.h \
    ../lib/OrderedHash.h \
	../lib/XmlHelper.h \
	Stable.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf


include(../qtservice/src/qtservice.pri)
