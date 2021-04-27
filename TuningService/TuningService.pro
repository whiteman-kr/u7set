QT += core
QT -= gui
QT += network
QT += qml
QT += xml
QT += widgets

TARGET = TuningSrv
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
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
    ../lib/BuildInfo.cpp \
    ../lib/DataSource.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/AppSignal.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Times.cpp \
	TuningService.cpp \
	TcpTuningServer.cpp \
	TcpTuningClient.cpp \
	TuningSource.cpp \
	TuningMemory.cpp \
	TuningClientContext.cpp \
	TuningServiceMain.cpp \
	TuningSourceThread.cpp \
	TuningDataStorage.cpp


HEADERS += \
	Stable.h \
    ../lib/BuildInfo.h \
    ../lib/OrderedHash.h \
    ../lib/DataSource.h \
	../lib/SoftwareSettings.h \
    ../lib/AppSignal.h \
    ../lib/TuningValue.h \
    ../lib/Times.h \
	../CommonLib/PropertyObject.h \
	TuningService.h \
	TcpTuningServer.h \
	TcpTuningClient.h \
	TuningSource.h \
	TuningMemory.h \
	TuningClientContext.h \
	TuningSourceThread.h \
	TuningDataStorage.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.


# DESTDIR
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L../bin/debug
	CONFIG(release, debug|release): LIBS += -L../bin/release
}
unix {
    CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release
}

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# ServiceLib
#
LIBS += -lServiceLib

# UtilsLib
#
LIBS += -lUtilsLib

# OnlineLib
#
LIBS += -lOnlineLib

# CommonLib
#
LIBS += -lCommonLib
