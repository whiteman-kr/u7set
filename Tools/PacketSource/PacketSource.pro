#-------------------------------------------------
#
# Project created by QtCreator 2018-03-27T13:22:36
#
#-------------------------------------------------

QT       += core gui widgets network sql qml xml

TARGET = PacketSource
TEMPLATE = app

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet


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
MainWindow.cpp \
Options.cpp \
SourceWorker.cpp \
SourceBase.cpp \
../../lib/XmlHelper.cpp \
../../lib/SocketIO.cpp \
../../lib/HostAddressPort.cpp \
../../lib/PlainObjectHeap.cpp \
../../lib/SimpleThread.cpp \
../../lib/Crc.cpp \
../../lib/DataProtocols.cpp \
../../lib/WUtils.cpp \
    ../../lib/Ui/DialogAbout.cpp \
    SignalBase.cpp \
    FrameBase.cpp \
    ../../lib/Signal.cpp \
    ../../lib/Address16.cpp \
    ../../lib/AppSignalStateFlags.cpp \
    ../../lib/DbStruct.cpp \
    ../../lib/DeviceObject.cpp \
    ../../lib/TuningValue.cpp \
    ../../lib/Types.cpp \
    ../../lib/ProtoSerialization.cpp \
	../../Proto/network.pb.cc \
	../../Proto/serialization.pb.cc \
    ../../lib/PropertyObject.cpp \
    ../../lib/ModuleFirmware.cpp \
    ../../lib/SignalProperties.cpp \
    ../../lib/DataSource.cpp \
    ../../Builder/IssueLogger.cpp \
    ../../lib/OutputLog.cpp \
    ../../lib/DeviceHelper.cpp \
    ../../lib/SimpleMutex.cpp \
    ../../lib/Times.cpp \
    PathOptionDialog.cpp \
    ../../lib/Address16.cpp


HEADERS += \
MainWindow.h \
Options.h \
SourceWorker.h \
SourceBase.h \
../../lib/XmlHelper.h \
../../lib/SocketIO.h \
../../lib/HostAddressPort.h \
../../lib/SimpleThread.h \
../../lib/Crc.h \
../../lib/DataProtocols.h \
../../lib/WUtils.h \
../../Builder/CfgFiles.h \
    ../../lib/Ui/DialogAbout.h \
    SignalBase.h \
    FrameBase.h \
    ../../lib/Signal.h \
    ../../lib/Address16.h \
    ../../lib/AppSignalStateFlags.h \
    ../../lib/DbStruct.h \
    ../../lib/DeviceObject.h \
    ../../lib/Hash.h \
    ../../lib/TuningValue.h \
    ../../lib/Types.h \
    ../../lib/ProtoSerialization.h \
	../../Proto/network.pb.h \
	../../Proto/serialization.pb.h \
    ../../lib/PropertyObject.h \
    ../../lib/Factory.h \
    ../../lib/ModuleFirmware.h \
    ../../lib/DebugInstCounter.h \
    ../../lib/OrderedHash.h \
    ../../lib/SignalProperties.h \
    ../../lib/DataSource.h \
    ../../Builder/IssueLogger.h \
    ../../lib/OutputLog.h \
    ../../lib/DeviceHelper.h \
    ../../lib/SimpleMutex.h \
    ../../lib/Times.h \
    PathOptionDialog.h \
    ../../lib/Address16.h

RESOURCES += \
resources.qrc


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11


# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# _DEBUG define, Windows memmory detection leak depends on it
#
CONFIG(debug, debug|release): DEFINES += _DEBUG

# Visual Leak Detector
#
#win32 {
#contains(QMAKE_TARGET.arch, x86_64) {
#LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
#LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
#} else {
#LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
#LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
#}

#INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
#INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
#}

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
	../../Proto/serialization.proto \
