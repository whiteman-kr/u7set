#-------------------------------------------------
#
# Project created by QtCreator 2016-05-09T01:22:58
#
#-------------------------------------------------

QT  += core gui qml xml
QT  += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TuningIPEN
TEMPLATE = app

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

include(../warnings.pri)

SOURCES +=\
    ../lib/Address16.cpp \
    ../lib/LanControllerInfoHelper.cpp \
	../lib/MemLeaksDetection.cpp \
	TuningMainWindow.cpp \
	../lib/SoftwareSettings.cpp \
	../lib/DeviceHelper.cpp \
	../lib/XmlHelper.cpp \
	../lib/DeviceObject.cpp \
	../lib/OutputLog.cpp \
	../lib/DbStruct.cpp \
	../Proto/serialization.pb.cc \
	../lib/Types.cpp \
	../lib/ProtoSerialization.cpp \
	../lib/Service.cpp \
	../lib/DataProtocols.cpp \
	../lib/DataSource.cpp \
	../lib/SimpleThread.cpp \
	../lib/SocketIO.cpp \
	../lib/UdpSocket.cpp \
	../lib/CircularLogger.cpp \
	../lib/Queue.cpp \
	MainIPEN.cpp \
	../lib/Signal.cpp \
	SafetyChannelSignalsModel.cpp \
	../lib/Crc.cpp \
	AnalogSignalSetter.cpp \
    ../lib/WUtils.cpp \
    TuningIPENService.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    DiscreteSignalSetter.cpp \
    TripleChannelSignalsModel.cpp \
    TuningIPENSocket.cpp \
    TuningIPENSource.cpp \
    ../Builder/ModulesRawData.cpp \
    TuningIPENDataStorage.cpp \
    ../TuningService/TuningDataStorage.cpp \
    ../lib/CommandLineParser.cpp \
    ../lib/AppSignal.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    AppSignalStateEx.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
	../lib/AppSignalStateFlags.cpp \
    ../lib/WidgetUtils.cpp \
    ../lib/SimpleMutex.cpp

HEADERS  += TuningMainWindow.h \
    ../lib/Address16.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
	../lib/MemLeaksDetection.h \
	../lib/SoftwareSettings.h \
	../lib/DeviceHelper.h \
	../lib/XmlHelper.h \
	../lib/DeviceObject.h \
	../lib/PropertyObject.h \
	../lib/OutputLog.h \
	../lib/DbStruct.h \
	../Proto/serialization.pb.h \
	../lib/Types.h \
	../lib/ProtoSerialization.h \
	../lib/Service.h \
	../lib/DataProtocols.h \
	../lib/DataSource.h \
	../lib/SimpleThread.h \
	../lib/SocketIO.h \
	../lib/UdpSocket.h \
	../lib/CircularLogger.h \
	../lib/Queue.h \
	../lib/Signal.h \
	SafetyChannelSignalsModel.h \
	../lib/Crc.h \
	AnalogSignalSetter.h \
    ../lib/WUtils.h \
	Stable.h \
    TuningIPENService.h \
    ../Builder/IssueLogger.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    DiscreteSignalSetter.h \
    TripleChannelSignalsModel.h \
    TuningIPENSocket.h \
    TuningIPENSource.h \
    ../Builder/ModulesRawData.h \
    TuningIPENDataStorage.h \
    ../TuningService/TuningDataStorage.h \
    ../lib/CommandLineParser.h \
    ../lib/AppSignal.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    AppSignalStateEx.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
	../lib/AppSignalStateFlags.h \
    ../lib/WidgetUtils.h \
    ../lib/SimpleMutex.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

include(../qtservice/src/qtservice.pri)

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

CONFIG(debug, debug|release) {
	OBJECTS_DIR = debug
	MOC_DIR = debug/moc
	RCC_DIR = debug/rcc
	UI_DIR = debug/ui
}

CONFIG(release, debug|release) {
	OBJECTS_DIR = release
	MOC_DIR = release/moc
	RCC_DIR = release/rcc
	UI_DIR = release/ui
}


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11
win32:QMAKE_CXXFLAGS += /std:c++17

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS		# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why

win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

RESOURCES += \
    TuningIPEN.qrc
