QT  += core
QT  += network
QT  += qml
QT  += concurrent

TARGET = PacketSourceConsole
CONFIG += console
TEMPLATE = app

DEFINES += Q_CONSOLE_APP

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

include(../../warnings.pri)

#Application icon
win32:RC_ICONS += ../PacketSource/icons/PacketSource.ico

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
    ../../Proto/network.pb.cc \
    ../../Proto/serialization.pb.cc \
    ../../lib/CfgServerLoader.cpp \
    ../../lib/MemLeaksDetection.cpp \
    ../../lib/CommandLineParser.cpp \
    ../../lib/HostAddressPort.cpp \
    ../../lib/SimpleThread.cpp \
    ../../lib/CircularLogger.cpp \
    ../../lib/SocketIO.cpp \
    ../../lib/BuildInfo.cpp \
    ../../lib/SoftwareSettings.cpp \
    ../../lib/TcpFileTransfer.cpp \
    ../../lib/Types.cpp \
    ../../lib/DataProtocols.cpp \
    ../../lib/WUtils.cpp \
    ../../lib/XmlHelper.cpp \
    ../../lib/Crc.cpp \
    ../../lib/DataSource.cpp \
    ../../Builder/IssueLogger.cpp \
    ../../lib/OutputLog.cpp \
    ../../lib/Signal.cpp \
    ../../lib/SignalProperties.cpp \
    ../../lib/TuningValue.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../lib/DeviceObject.cpp \
    ../../lib/ModuleFirmware.cpp \
    ../../lib/ProtoSerialization.cpp \
    ../../lib/DbStruct.cpp \
    ../../lib/SimpleMutex.cpp \
    ../../lib/Times.cpp \
    ../../lib/SoftwareInfo.cpp \
    ../../lib/Address16.cpp \
    ../../lib/Tcp.cpp \
    ../PacketSource/ConfigSocket.cpp \
    ../PacketSource/CmdLineParam.cpp \
	../PacketSource/BuildOption.cpp \
    ../PacketSource/SourceBase.cpp \
    ../PacketSource/SignalBase.cpp \
    ../PacketSource/History.cpp \
    ../PacketSource/FrameBase.cpp \
    ../PacketSource/SourceWorker.cpp \
    ../PacketSource/UalTesterServer.cpp \
    ../PacketSource/PacketSourceCore.cpp

HEADERS += \
    ../../Proto/network.pb.h \
    ../../Proto/serialization.pb.h \
    ../../Builder/CfgFiles.h \
    ../../lib/CfgServerLoader.h \
    ../../lib/MemLeaksDetection.h \
    ../../lib/CommandLineParser.h \
    ../../lib/OrderedHash.h \
    ../../lib/HostAddressPort.h \
    ../../lib/SimpleThread.h \
    ../../lib/CircularLogger.h \
    ../../lib/SocketIO.h \
    ../../lib/BuildInfo.h \
    ../../lib/Socket.h \
    ../../lib/SoftwareSettings.h \
    ../../lib/TcpFileTransfer.h \
    ../../lib/Types.h \
    ../../lib/DataProtocols.h \
    ../../lib/WUtils.h \
    ../../lib/XmlHelper.h \
    ../../lib/Crc.h \
    ../../lib/DataSource.h \
    ../../Builder/IssueLogger.h \
    ../../lib/OutputLog.h \
    ../../lib/Signal.h \
    ../../lib/SignalProperties.h \
    ../../lib/TuningValue.h \
    ../../lib/DeviceHelper.h \
    ../../lib/DeviceObject.h \
    ../../lib/DebugInstCounter.h \
    ../../lib/Factory.h \
    ../../lib/ModuleFirmware.h \
    ../../lib/PropertyObject.h \
    ../../lib/ProtoSerialization.h \
    ../../lib/DbStruct.h \
    ../../lib/SimpleMutex.h \
    ../../lib/Times.h \
    ../../lib/SoftwareInfo.h \
    ../../lib/Address16.h \
    ../../lib/Tcp.h \
    ../PacketSource/CmdLineParam.h \
	../PacketSource/BuildOption.h \
    ../PacketSource/ConfigSocket.h \
    ../PacketSource/SourceBase.h \
    ../PacketSource/SignalBase.h \
    ../PacketSource/History.h \
    ../PacketSource/FrameBase.h \
    ../PacketSource/SourceWorker.h \
    ../PacketSource/UalTesterServer.h \
    ../PacketSource/PacketSourceCore.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


RESOURCES += \
    Resources.qrc


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



