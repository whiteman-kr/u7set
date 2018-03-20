#-------------------------------------------------
#
# Project created by QtCreator 2014-11-18T12:51:14
#
#-------------------------------------------------

QT       += core network qml xml

QT       -= gui

TARGET = CfgSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

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

#c++14/17 support
#
CONFIG += c++14
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

# Force prebuild version control info
#
win32 {
	contains(QMAKE_TARGET.arch, x86_64){
		QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
		system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
			qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
			nmake))
		system(chdir $$PWD & \
			$$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/ConfigurationService.pro)
	}
	else{
		QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
		system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
			qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
			nmake))
		system(chdir $$PWD & \
			$$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/ConfigurationService.pro)
	}
}
unix {
	QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
	system(cd $$PWD/../GetGitProjectVersion; \
		qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
		make;)
	system(cd $$PWD; \
		$$PWD/../bin_unix/GetGitProjectVersion $$PWD/ConfigurationService.pro)
}


SOURCES += \
    ../lib/CfgServerLoader.cpp \
    ../lib/Tcp.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SocketIO.cpp \
    ConfigurationService.cpp \
	../lib/Service.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/CommandLineParser.cpp \
    CfgServiceMain.cpp \
    ../lib/XmlHelper.cpp \
    CfgChecker.cpp \
    CfgControlServer.cpp \
    ../lib/SoftwareInfo.cpp

HEADERS += \
    version.h \
    ../lib/CfgServerLoader.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/SimpleThread.h \
    ../lib/BuildInfo.h \
    ../lib/SocketIO.h \
    ConfigurationService.h \
	../lib/Service.h \
    ../lib/UdpSocket.h \
    ../lib/CircularLogger.h \
    ../lib/JsonSerializable.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/CommandLineParser.h \
    ../lib/XmlHelper.h \
    CfgChecker.h \
    CfgControlServer.h \
    ../lib/Types.h \
    ../lib/SoftwareInfo.h

include(../qtservice/src/qtservice.pri)

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

DISTFILES += \
    ../Proto/network.proto
