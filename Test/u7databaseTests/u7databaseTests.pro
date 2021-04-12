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
	../../lib/DbController.cpp \
	../../lib/DbWorker.cpp \
	../../lib/DbStruct.cpp \
	../../lib/DeviceObject.cpp \
	../../lib/DbProgress.cpp \
    ../../lib/ScriptDeviceObject.cpp \
	../../lib/Signal.cpp \
	../../lib/DbProgressDialog.cpp \
	../../lib/Types.cpp \
	../../lib/TuningValue.cpp \
	../../lib/SignalProperties.cpp \
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

#    SignalTests.h \

HEADERS += \
    ../../lib/DbController.h \
	../../lib/DbWorker.h \
	../../lib/DbStruct.h \
	../../lib/DeviceObject.h \
	../../lib/Factory.h \
	../../lib/DbProgress.h \
    ../../lib/ScriptDeviceObject.h \
	../../lib/Signal.h \
	../../lib/DbProgressDialog.h \
	../../lib/PropertyObject.h \
	../../lib/Types.h \
	../../lib/TuningValue.h \
	../../lib/SignalProperties.h \
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

