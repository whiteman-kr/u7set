
QT  += core
QT  -= gui
QT  += network
QT  += qml

TARGET = UalTester
CONFIG += console
CONFIG -= app_bundle


TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../../warnings.pri)

#Application icon
win32:RC_ICONS += icons/UalTester.ico

# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

SOURCES += \
	../../lib/BuildInfo.cpp \
	../../lib/SoftwareSettings.cpp \
	../../lib/AppSignalParam.cpp \
	../../lib/AppSignal.cpp \
	../../lib/Tuning/TuningSignalState.cpp \
	../../lib/TuningValue.cpp \
	main.cpp \
	UalTester.cpp \
	SignalBase.cpp \
	TestFile.cpp \
	TuningSocket.cpp \
	CmdLineParam.cpp \
	TuningSourceBase.cpp \
	SignalStateSocket.cpp \

HEADERS += \
	Stable.h \
	../../lib/ConstStrings.h \
    ../../lib/OrderedHash.h \
    ../../lib/BuildInfo.h \
	../../lib/SoftwareSettings.h \
	../../CommonLib/PropertyObject.h \
    ../../lib/AppSignalParam.h \
    ../../lib/AppSignal.h \
    ../../lib/Tuning/TuningSignalState.h \
    ../../lib/TuningValue.h \
    SignalBase.h \
    TestFile.h \
    TuningSocket.h \
	CmdLineParam.h \
	TuningSourceBase.h \
	UalTester.h \
	SignalStateSocket.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

RESOURCES += \
    Resources.qrc

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

DISTFILES += \
	../../Proto/network.proto \
	../../Proto/serialization.proto

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
    CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}


LIBS += -lOnlineLib
LIBS += -lServiceLib
LIBS += -lUtilsLib
LIBS += -lCommonLib

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../../Protobuf




