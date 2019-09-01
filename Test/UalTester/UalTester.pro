
QT  += core
QT  -= gui
QT  += network
QT  += qml

TARGET = UalTester
CONFIG += console
CONFIG -= app_bundle


TEMPLATE = app

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

include(../warnings.pri)

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
        main.cpp \
    UalTester.cpp \
    ../../lib/CommandLineParser.cpp \
    ../../lib/CircularLogger.cpp \
    ../../lib/SimpleThread.cpp \
	../../lib/WUtils.cpp \
    ../../lib/HostAddressPort.cpp \
    ../../lib/SocketIO.cpp \
    ../../lib/CfgServerLoader.cpp \
    ../../Proto/network.pb.cc \
    ../../Proto/serialization.pb.cc \
    ../../lib/BuildInfo.cpp \
    ../../lib/Tcp.cpp \
    ../../lib/TcpFileTransfer.cpp \
    ../../lib/Types.cpp \
    ../../lib/ProtoSerialization.cpp \
    ../../lib/SoftwareInfo.cpp \
	../../lib/XmlHelper.cpp \
    ../../lib/ServiceSettings.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../Builder/IssueLogger.cpp \
    ../../lib/DeviceObject.cpp \
    ../../Builder/ModulesRawData.cpp \
    ../../lib/OutputLog.cpp \
    ../../lib/DbStruct.cpp \
	../../lib/AppSignal.cpp \
	SignalStateSocket.cpp \
	../../lib/Signal.cpp \
	../../lib/Tuning/TuningSignalState.cpp \
	../../lib/TuningValue.cpp \
	../../lib/SignalProperties.cpp \
	SignalBase.cpp \
	TestFile.cpp \
	TuningSocket.cpp \
	TuningSignalBase.cpp \
	CmdLineParam.cpp \
	../../lib/MemLeaksDetection.cpp


HEADERS += \
	UalTester.h \
    ../../lib/CommandLineParser.h \
    ../../lib/OrderedHash.h \
    ../../lib/CircularLogger.h \
    ../../lib/SimpleThread.h \
	../../lib/WUtils.h \
    ../../lib/HostAddressPort.h \
    ../../lib/SocketIO.h \
    ../../lib/CfgServerLoader.h \
    ../../Proto/network.pb.h \
    ../../Proto/serialization.pb.h \
    ../../lib/BuildInfo.h \
    ../../lib/Tcp.h \
    ../../lib/TcpFileTransfer.h \
    ../../lib/Types.h \
    ../../lib/ProtoSerialization.h \
    ../../lib/SoftwareInfo.h \
	../../lib/XmlHelper.h \
    ../../lib/ServiceSettings.h \
    ../../lib/DeviceHelper.h \
    ../../Builder/IssueLogger.h \
    ../../lib/DeviceObject.h \
    ../../Builder/ModulesRawData.h \
    ../../lib/OutputLog.h \
    ../../lib/DbStruct.h \
    ../../lib/PropertyObject.h \
    ../../lib/AppSignal.h \
    ../../lib/Hash.h \
    SignalStateSocket.h \
    ../../lib/Signal.h \
    ../../lib/Tuning/TuningSignalState.h \
    ../../lib/TuningValue.h \
    ../../lib/SignalProperties.h \
    SignalBase.h \
    TestFile.h \
    TuningSocket.h \
    TuningSignalBase.h \
	CmdLineParam.h \
	Stable.h \
	../../lib/MemLeaksDetection.h


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
		LIBS += -L$$DESTDIR -lprotobuf

		INCLUDEPATH += ./../../Protobuf
}
unix {
		LIBS += -lprotobuf
}

DISTFILES += \
	../../Proto/network.proto \
	../../Proto/serialization.proto
