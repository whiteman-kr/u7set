
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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES += \
	../../lib/BuildInfo.cpp \
	../../lib/SoftwareSettings.cpp \
	../../lib/Tuning/TuningSignalState.cpp \
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
    ../../lib/Tuning/TuningSignalState.h \
    SignalBase.h \
    TestFile.h \
    TuningSocket.h \
	CmdLineParam.h \
	TuningSourceBase.h \
	UalTester.h \
	SignalStateSocket.h \

RESOURCES += \
    Resources.qrc

DISTFILES += \
    ../../Proto/network.proto \
	../../Proto/serialization.proto

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib

LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib

LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib

LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib

LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
INCLUDEPATH += ./../../Protobuf

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
