#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T14:26:26
#
#-------------------------------------------------

QT       += core gui widgets network xmlpatterns qml xml svg

TARGET = Monitor
TEMPLATE = app

INCLUDEPATH += $$PWD

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


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES += main.cpp \
    MonitorMainWindow.cpp \
	MonitorCentralWidget.cpp \
	Settings.cpp \
    ../lib/SocketIO.cpp \
    DialogSettings.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Tcp.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SimpleThread.cpp \
    MonitorSchemaView.cpp \
    MonitorSchemaWidget.cpp \
    ../lib/Types.cpp \
    MonitorConfigController.cpp \
    SchemaManager.cpp \
    ../Proto/network.pb.cc \
    TcpSignalClient.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/Signal.cpp \
    ../lib/PropertyObject.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtobufHelper.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/AppSignalState.cpp \
    ../lib/AppSignalManager.cpp \
    Statistics.cpp \
    DialogSignalInfo.cpp \
    DialogSignalSearch.cpp \
    DialogSignalSnapshot.cpp

HEADERS  += \
    MonitorMainWindow.h \
    MonitorCentralWidget.h \
	Stable.h \
	Settings.h \
    ../lib/SocketIO.h \
    DialogSettings.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
    MonitorSchemaView.h \
    MonitorSchemaWidget.h \
    ../lib/Types.h \
    MonitorConfigController.h \
    SchemaManager.h \
    ../Proto/network.pb.h \
    TcpSignalClient.h \
    ../lib/Hash.h \
    ../Proto/serialization.pb.h \
    ../lib/Signal.h \
    ../lib/PropertyObject.h \
    ../lib/XmlHelper.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ProtobufHelper.h \
    ../lib/ProtoSerialization.h \
    ../lib/AppSignalState.h \
    ../lib/AppSignalManager.h \
    Statistics.h \
    DialogSignalInfo.h \
    DialogSignalSearch.h \
    DialogSignalSnapshot.h

FORMS    += \
    DialogSettings.ui \
    DialogSignalInfo.ui \
    DialogSignalSearch.ui \
    DialogSignalSnapshot.ui


# Optimization flags
#
win32 {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}


# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# _DEBUG define, Windows memmory detection leak depends on it
#
CONFIG(debug, debug|release): DEFINES += _DEBUG

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11


# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

RESOURCES += \
    Monitor.qrc

DISTFILES += \
    Images/NewSchema.svg \
    Images/Backward.svg \
    Images/Forward.svg \
    Images/ZoomIn.svg \
    Images/ZoomOut.svg \
    Images/Settings.svg \
    Images/Close.svg \
    Images/About.svg \
    Images/readme.txt \
    ../Proto/network.proto \
    ../Proto/serialization.proto
