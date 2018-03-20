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

#c++14/17 support
#
CONFIG += c++14
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet


SOURCES +=\
	TuningMainWindow.cpp \
	../lib/ServiceSettings.cpp \
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
	../lib/JsonSerializable.cpp \
	MainIPEN.cpp \
	../lib/Signal.cpp \
	SafetyChannelSignalsModel.cpp \
	../AppDataService/AppSignalStateEx.cpp \
	../lib/Crc.cpp \
	AnalogSignalSetter.cpp \
    ../lib/WUtils.cpp \
    TuningIPENService.cpp \
    ../u7/Builder/IssueLogger.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    DiscreteSignalSetter.cpp \
    TripleChannelSignalsModel.cpp \
    TuningIPENSocket.cpp \
    TuningIPENSource.cpp \
    ../u7/Builder/ModulesRawData.cpp \
    TuningIPENDataStorage.cpp \
    ../TuningService/TuningDataStorage.cpp \
    ../lib/CommandLineParser.cpp \
    ../lib/AppSignal.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp

HEADERS  += TuningMainWindow.h \
	../lib/ServiceSettings.h \
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
	../lib/JsonSerializable.h \
	../lib/Signal.h \
	SafetyChannelSignalsModel.h \
	../AppDataService/AppSignalStateEx.h \
	../lib/Crc.h \
	AnalogSignalSetter.h \
    ../lib/WUtils.h \
    TuningIPENService.h \
    ../u7/Builder/IssueLogger.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    DiscreteSignalSetter.h \
    TripleChannelSignalsModel.h \
    TuningIPENSocket.h \
    TuningIPENSource.h \
    ../u7/Builder/ModulesRawData.h \
    TuningIPENDataStorage.h \
    ../TuningService/TuningDataStorage.h \
    ../lib/CommandLineParser.h \
    ../lib/AppSignal.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h

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

# Force prebuild version control info
#
win32 {
        contains(QMAKE_TARGET.arch, x86_64){
                QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
                system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
                        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
                        nmake))
                system(chdir $$PWD & \
                        $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/TuningIPEN.pro)
        }
        else{
                QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
                system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
                        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
                        nmake))
                system(chdir $$PWD & \
                        $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/TuningIPEN.pro)
        }
}
unix {
        QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
        system(cd $$PWD/../GetGitProjectVersion; \
                qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
                make;)
        system(cd $$PWD; \
                $$PWD/../bin_unix/GetGitProjectVersion $$PWD/TuningIPEN.pro)
}


#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11


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
