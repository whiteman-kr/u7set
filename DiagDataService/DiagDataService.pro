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
    ../lib/Types.cpp \
    ../lib/BuildInfo.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/SoftwareInfo.cpp \
	DiagDataService.cpp \
	DiagDataServiceMain.cpp \

HEADERS += \
	Stable.h \
    ../lib/Types.h \
    ../lib/BuildInfo.h \
	../lib/SoftwareSettings.h \
    ../lib/SoftwareInfo.h \
	DiagDataService.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

INCLUDEPATH += ./../Protobuf

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

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
LIBS += -lprotobuf

