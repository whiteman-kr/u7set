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


SOURCES += MainWindow.cpp \
    ../lib/Address16.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/ScriptDeviceObject.cpp \
	../lib/SoftwareSettings.cpp \
    ScanOptionsWidget.cpp \
	ScmMain.cpp \
	ScmTcpAppDataClient.cpp \
    ServiceTableModel.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    ../lib/DataSource.cpp \
    BaseServiceStateWidget.cpp \
    ConfigurationServiceWidget.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/Queue.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/WUtils.cpp \
    ../lib/Crc.cpp \
    ../lib/Tcp.cpp \
    ../lib/Signal.cpp \
    ../lib/AppSignal.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/Types.cpp \
    ../lib/DbStruct.cpp \
    ../lib/HostAddressPort.cpp \
    ../AppDataService/AppDataSource.cpp \
    ../lib/CircularLogger.cpp \
    TcpConfigServiceClient.cpp \
    AppDataServiceWidget.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/Service.cpp \
    ../lib/CommandLineParser.cpp \
    ../lib/TuningValue.cpp \
    TuningServiceWidget.cpp \
    TcpTuningServiceClient.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
    TuningSourceWidget.cpp \
    ../lib/OutputLog.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/DeviceHelper.cpp \
    ../Builder/ModulesRawData.cpp \
    AppDataSourceWidget.cpp \
    ../lib/Times.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/SignalProperties.cpp \
    ../AppDataService/RtTrendsServer.cpp \
	../lib/AppSignalStateFlags.cpp \
    ../lib/WidgetUtils.cpp \
    ../lib/SimpleMutex.cpp \
    ../lib/SimpleAppSignalState.cpp \
    ../AppDataService/DynamicAppSignalState.cpp

HEADERS  += MainWindow.h \
    ../lib/Address16.h \
    ../lib/CUtils.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
    ../lib/ScriptDeviceObject.h \
    ../lib/SoftwareSettings.h \
    ScanOptionsWidget.h \
    ScmTcpAppDataClient.h \
    ServiceTableModel.h \
    ../lib/UdpSocket.h \
    ../lib/SocketIO.h \
    ../lib/DataSource.h \
    BaseServiceStateWidget.h \
    ConfigurationServiceWidget.h \
    ../lib/SimpleThread.h \
    ../lib/XmlHelper.h \
    ../lib/Queue.h \
    ../lib/DataProtocols.h \
    ../lib/WUtils.h \
    ../lib/Crc.h \
    Stable.h \
    ../lib/Tcp.h \
    ../lib/Signal.h \
    ../lib/PropertyObject.h \
	 ../lib/AppSignal.h \
    ../lib/DeviceObject.h \
    ../lib/Types.h \
    ../lib/DbStruct.h \
    ../lib/HostAddressPort.h \
    ../AppDataService/AppDataSource.h \
    ../lib/CircularLogger.h \
    TcpConfigServiceClient.h \
    ../lib/WidgetUtils.h \
    AppDataServiceWidget.h \
    ../lib/SoftwareInfo.h \
    ../lib/OrderedHash.h \
    ../lib/Service.h \
    ../lib/CommandLineParser.h \
    ../lib/TuningValue.h \
    TuningServiceWidget.h \
    TcpTuningServiceClient.h \
    ../lib/Tuning/TuningSourceState.h \
    TuningSourceWidget.h \
    ../lib/OutputLog.h \
    ../Builder/IssueLogger.h \
    ../lib/DeviceHelper.h \
    ../Builder/ModulesRawData.h \
    AppDataSourceWidget.h \
    ../lib/Times.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/SignalProperties.h \
    ../AppDataService/RtTrendsServer.h \
    ../lib/AppSignalStateFlags.h \
    ../lib/SimpleMutex.h \
    ../lib/SimpleAppSignalState.h \
    ../AppDataService/DynamicAppSignalState.h

include(../qtservice/src/qtservice.pri)

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
