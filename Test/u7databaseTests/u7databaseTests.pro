#-------------------------------------------------
#
# Project created by QtCreator 2015-06-23T12:45:26
#
#-------------------------------------------------

QT       += core sql testlib network widgets xml qml
QT       -= gui

TARGET = u7databasetests
CONFIG   += console coverage
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += Q_CONSOLE_APP
DEFINES += SRCDIR=\\\"$$PWD/\\\"

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

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

CONFIG += warn_off

#	SignalTests.cpp \

SOURCES += main.cpp \
    DeviceObjectTests.cpp \
	UserPropertyTest.cpp \
	UserTests.cpp \
	FileTests.cpp \
	OtherTests.cpp \
	PropertyObjectTests.cpp \
	ProjectPropertyTests.cpp \
	DbControllerFileManagementTests.cpp \
	DbControllerHardwareConfigurationTests.cpp \
	DbControllerVersionControlTests.cpp \
	DbControllerSignalManagementTests.cpp \
	TestDbBase.cpp \
	DbControllerUserTests.cpp \
	DbControllerProjectTests.cpp

HEADERS += \
    DeviceObjectTests.h \
	UserPropertyTest.h \
	UserTests.h \
	FileTests.h \
	OtherTests.h \
	PropertyObjectTests.h \
	ProjectPropertyTests.h \
	DbControllerFileManagementTests.h \
	DbControllerHardwareConfigurationTests.h \
	DbControllerVersionControlTests.h \
	DbControllerSignalManagementTests.h \
	TestDbBase.h \
	DbControllerUserTests.h \
	DbControllerProjectTests.h

# Protobuf
#
LIBS += -L$$DESTDIR/. -lprotobuf
INCLUDEPATH += $$PWD/../../Protobuf

DISTFILES += \
    ../../Proto/serialization.proto

RESOURCES += \
    ../../DatabaseUpgrade/DatabaseUpgrade.qrc \
    FutureDatabaseUpgrade.qrc

# --
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

LIBS += -L$$DESTDIR
LIBS += -L.

LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

LIBS += -lHardwareLib
win32:PRE_TARGETDEPS += $$DESTDIR/HardwareLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libHardwareLib.a

LIBS += -lDbLib
win32:PRE_TARGETDEPS += $$DESTDIR/DbLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libDbLib.a

LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a

LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
