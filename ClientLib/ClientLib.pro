QT += xml network widgets

TARGET = ClientLib

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD
INCLUDEPATH += ./../Protobuf

include(../compiler.pri)
include(../warnings.pri)
include(../codecoverage.pri)

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
    AdsConnection.h \
	AdsSourceStateConnection.h \
	AppSignalManager.h \
    ConfigController.h \
    IAppSignalUpdater.h \
    IRecentAppSignals.h \
	Stable.h \
	TcpAppSourcesState.h \
	TcpSignalClient.h \
	TcpSignalRecents.h \
    TuningSourceState.h \
    TuningSourcesHelper.h \
    TuningTcpClient.h \
    TuningUserManager.h

SOURCES += \
    AdsConnection.cpp \
	AdsSourceStateConnection.cpp \
	AppSignalManager.cpp \
	ConfigController.cpp \
	TcpAppSourcesState.cpp \
	TcpSignalClient.cpp \
	TcpSignalRecents.cpp \
    TuningSourceState.cpp \
    TuningSourcesHelper.cpp \
    TuningTcpClient.cpp \
    TuningUserManager.cpp

DISTFILES += \

