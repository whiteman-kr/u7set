#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T13:33:03
#
#-------------------------------------------------

QT       += core gui network qml xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scm
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
    ../lib/Address16.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/ScriptDeviceObject.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/DataSource.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/Queue.cpp \
    ../lib/WUtils.cpp \
    ../lib/Crc.cpp \
    ../lib/Signal.cpp \
    ../lib/AppSignal.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Types.cpp \
    ../lib/DbStruct.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/Times.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/SignalProperties.cpp \
	../lib/AppSignalStateFlags.cpp \
    ../lib/WidgetUtils.cpp \
    ../lib/SimpleMutex.cpp \
	../Builder/IssueLogger.cpp \
	../Builder/ModulesRawData.cpp \
	../AppDataService/DynamicAppSignalState.cpp \
	../AppDataService/AppDataSource.cpp \
	../AppDataService/RtTrendsServer.cpp \
	MainWindow.cpp \
	ScanOptionsWidget.cpp \
	ScmMain.cpp \
	ScmTcpAppDataClient.cpp \
	ServiceTableModel.cpp \
	BaseServiceStateWidget.cpp \
	ConfigurationServiceWidget.cpp \
	TcpConfigServiceClient.cpp \
	AppDataServiceWidget.cpp \
	TuningServiceWidget.cpp \
	TcpTuningServiceClient.cpp \
	TuningSourceWidget.cpp \
	AppDataSourceWidget.cpp \

HEADERS  += \
	Stable.h \
    ../lib/Address16.h \
    ../lib/CUtils.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/SoftwareSettings.h \
    ../lib/DataSource.h \
    ../lib/SimpleThread.h \
    ../lib/XmlHelper.h \
    ../lib/Queue.h \
    ../lib/WUtils.h \
    ../lib/Crc.h \
    ../lib/Signal.h \
    ../lib/PropertyObject.h \
	../lib/AppSignal.h \
    ../lib/DeviceObject.h \
    ../lib/Types.h \
    ../lib/DbStruct.h \
    ../lib/HostAddressPort.h \
    ../lib/WidgetUtils.h \
    ../lib/SoftwareInfo.h \
    ../lib/OrderedHash.h \
    ../lib/TuningValue.h \
    ../lib/Tuning/TuningSourceState.h \
    ../lib/OutputLog.h \
    ../lib/DeviceHelper.h \
    ../lib/Times.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/SignalProperties.h \
    ../lib/AppSignalStateFlags.h \
    ../lib/SimpleMutex.h \
	../Builder/IssueLogger.h \
	../Builder/ModulesRawData.h \
	../AppDataService/DynamicAppSignalState.h \
	../AppDataService/AppDataSource.h \
	../AppDataService/RtTrendsServer.h \
	MainWindow.h \
	AppDataServiceWidget.h \
	TuningServiceWidget.h \
	TcpTuningServiceClient.h \
	TuningSourceWidget.h \
	AppDataSourceWidget.h \
	TcpConfigServiceClient.h \
	ScanOptionsWidget.h \
	ScmTcpAppDataClient.h \
	ServiceTableModel.h \
	BaseServiceStateWidget.h \
	ConfigurationServiceWidget.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

FORMS    +=

TRANSLATIONS = ./translations/ServiceControlManager_ru.ts \
               ./translations/ServiceControlManager_uk.ts

RESOURCES += \
    ServiceControlManager.qrc


# Protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf

DISTFILES += \
    ../Proto/network.proto \
	../Proto/serialization.proto

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



