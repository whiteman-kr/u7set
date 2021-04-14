
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
    ../../lib/ScriptDeviceObject.cpp \
	../../lib/BuildInfo.cpp \
    ../../lib/Types.cpp \
    ../../lib/SoftwareInfo.cpp \
	../../lib/SoftwareSettings.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../lib/DeviceObject.cpp \
    ../../lib/OutputLog.cpp \
    ../../lib/DbStruct.cpp \
	../../lib/AppSignalParam.cpp \
	../../lib/AppSignal.cpp \
	../../lib/Tuning/TuningSignalState.cpp \
	../../lib/TuningValue.cpp \
	../../lib/SignalProperties.cpp \
	../../Builder/IssueLogger.cpp \
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
    ../../lib/ScriptDeviceObject.h \
    ../../lib/OrderedHash.h \
    ../../lib/BuildInfo.h \
    ../../lib/Types.h \
    ../../lib/SoftwareInfo.h \
	../../lib/SoftwareSettings.h \
    ../../lib/DeviceHelper.h \
    ../../lib/DeviceObject.h \
    ../../lib/OutputLog.h \
    ../../lib/DbStruct.h \
    ../../lib/PropertyObject.h \
    ../../lib/AppSignalParam.h \
    ../../lib/AppSignal.h \
    ../../lib/Tuning/TuningSignalState.h \
    ../../lib/TuningValue.h \
    ../../lib/SignalProperties.h \
	../../Builder/IssueLogger.h \
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

# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../../Protobuf


DISTFILES += \
	../../Proto/network.proto \
	../../Proto/serialization.proto

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# OnlineLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lOnlineLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lOnlineLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lOnlineLib
}

# ServiceLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lServiceLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lServiceLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lServiceLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lServiceLib
}

# UtilsLib
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lUtilsLib
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lUtilsLib
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lUtilsLib
}

