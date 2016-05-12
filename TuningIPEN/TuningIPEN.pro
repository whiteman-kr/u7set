#-------------------------------------------------
#
# Project created by QtCreator 2016-05-09T01:22:58
#
#-------------------------------------------------

QT  += core gui qml
QT  += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TuningIPEN
TEMPLATE = app


SOURCES +=\
        TuningMainWindow.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/PropertyObject.cpp \
    ../lib/OutputLog.cpp \
    ../lib/DbStruct.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/Types.cpp \
    ../lib/ProtoSerialization.cpp \
    ../TuningService/TuningDataSource.cpp \
    ../TuningService/TuningService.cpp \
    ../lib/Service.cpp \
    ../lib/DataProtocols.cpp \
    ../lib/DataSource.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/SocketIO.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Queue.cpp \
    ../lib/JsonSerializable.cpp \
    MainIPEN.cpp \
    ../u7/Builder/TuningDataStorage.cpp \
    ../lib/Signal.cpp \
    SafetyChannelSignalsModel.cpp
    ../AppDataService/AppSignalState.cpp

HEADERS  += TuningMainWindow.h \
    ../include/ServiceSettings.h \
    ../include/DeviceHelper.h \
    ../include/XmlHelper.h \
    ../include/DeviceObject.h \
    ../include/PropertyObject.h \
    ../include/OutputLog.h \
    ../include/DbStruct.h \
    ../Proto/serialization.pb.h \
    ../include/Types.h \
    ../include/ProtoSerialization.h \
    ../TuningService/TuningDataSource.h \
    ../TuningService/TuningService.h \
    ../include/Service.h \
    ../include/DataProtocols.h \
    ../include/DataSource.h \
    ../include/SimpleThread.h \
    ../include/SocketIO.h \
    ../include/UdpSocket.h \
    ../include/CircularLogger.h \
    ../include/Queue.h \
    ../include/JsonSerializable.h \
    ../u7/Builder/TuningDataStorage.h \
    ../include/Signal.h \
    SafetyChannelSignalsModel.h
    ../AppDataService/AppSignalState.h

FORMS    += TuningMainWindow.ui

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


CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# Visual Leak Detector
#
win32 {
	    contains(QMAKE_TARGET.arch, x86_64) {
			    LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win64"
			    LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	    } else {
			    LIBS += -L"C:/Program Files/Visual Leak Detector/lib/Win32"
			    LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win32"
	    }

	    INCLUDEPATH += "C:/Program Files/Visual Leak Detector/include"
	    INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
}

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
