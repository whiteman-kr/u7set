QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml
QT  += sql

TARGET = GatewaySrv
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../compiler.pri)
include(../warnings.pri)
include(../sanitizer.pri)
include(../codecoverage.pri)

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
	AppDataServiceClient.cpp \
	AppSignalState.cpp \
	GatewayDescription.cpp \
	GatewayDescriptionParser.cpp \
	AppSignalStates.cpp \
	GatewayHandler.cpp \
	GatewayService.cpp \
	GatewayServiceMain.cpp \
	IvsImpulseGatewayHandler.cpp

HEADERS += \
	../lib/ConstStrings.h \
	../lib/BuildInfo.h \
	AppDataServiceClient.h \
	AppSignalState.h \
	GatewayDescription.h \
	GatewayDescriptionParser.h \
	AppSignalStates.h \
	GatewayHandler.h \
	GatewayService.h \
	IvsImpulseGatewayHandler.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

INCLUDEPATH +=  ./../Protobuf \
				./../asio/include \

DEFINES += ASIO_STANDALONE=1

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto \
    GatewayDescription.txt

CONFIG(release, debug|release): unix:QMAKE_CXXFLAGS += -DNDEBUG

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a

# ServiceLib
#
LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libServiceLib.a

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a
