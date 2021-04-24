QT  += core
QT  -= gui
QT  += network
QT  += widgets
QT  += qml
QT  += xml

TARGET = DiagDataSrv
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
    ../lib/DataSource.cpp \
    ../lib/AppSignal.cpp \
    ../lib/BuildInfo.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
	DiagDataService.cpp \
	DiagDataServiceMain.cpp \

HEADERS += \
	Stable.h \
	../lib/LanControllerInfo.h \
    ../lib/DataSource.h \
    ../lib/AppSignal.h \
    ../lib/CUtils.h \
	../CommonLib/PropertyObject.h \
    ../lib/BuildInfo.h \
	../lib/SoftwareSettings.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
	DiagDataService.h \


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

win32 {
        CONFIG(debug, debug|release): LIBS += -L../bin/debug/
		CONFIG(release, debug|release): LIBS += -L../bin/release/
}
unix {
        CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/
		CONFIG(release, debug|release): LIBS += -L../bin_unix/release/
}

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# HardwareLib
#
LIBS += -lHardwareLib

# ServiceLib
#
LIBS += -lServiceLib

# OnlineLib
#
LIBS += -lOnlineLib

# UtilsLib
#
LIBS += -lUtilsLib

# CommonLib
#
LIBS += -lCommonLib

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf
