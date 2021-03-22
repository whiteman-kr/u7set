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
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest


#	SignalTests.cpp \

SOURCES += main.cpp \
    ../../lib/Address16.cpp \
	../../lib/DbController.cpp \
	../../lib/DbWorker.cpp \
	../../lib/DbStruct.cpp \
	../../lib/DeviceObject.cpp \
	../../lib/DbProgress.cpp \
    ../../lib/ScriptDeviceObject.cpp \
	../../lib/Signal.cpp \
	../../lib/DbProgressDialog.cpp \
	../../lib/Types.cpp \
	../../lib/XmlHelper.cpp \
	../../lib/HostAddressPort.cpp \
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
    ../../lib/Address16.h \
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
	../../lib/XmlHelper.h \
	../../lib/HostAddressPort.h \
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
