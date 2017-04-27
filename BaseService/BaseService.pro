QT      += core
QT      -= gui
QT      += network
QT	+= widgets
QT      += qml
QT      += xml

TARGET = BaseSrv
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


# Force prebuild version control info
#
win32 {
	contains(QMAKE_TARGET.arch, x86_64){
		QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
		system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
			qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
			nmake))
		system(chdir $$PWD & \
			$$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/BaseService.pro)
	}
	else{
		QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
		system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
			qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
			nmake))
		system(chdir $$PWD & \
			$$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/BaseService.pro)
	}
}
unix {
	QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
	system(cd $$PWD/../GetGitProjectVersion; \
		qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
		make;)
	system(cd $$PWD; \
		$$PWD/../bin_unix/GetGitProjectVersion $$PWD/BaseService.pro)
}


SOURCES += \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/UdpFileTransfer.cpp \
    ../lib/Service.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/CommandLineParser.cpp \
    ../lib/WUtils.cpp \
    BaseServiceMain.cpp

HEADERS += \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/CircularLogger.h \
	../lib/FscDataFormat.h \
    version.h \
    ../lib/JsonSerializable.h \
    ../lib/UdpFileTransfer.h \
    ../lib/Service.h \
    ../lib/SimpleThread.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/CommandLineParser.h \
    ../lib/WUtils.h

include(../qtservice/src/qtservice.pri)

CONFIG += c++14
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

#protobuf
#

win32 {
	LIBS += -L$$DESTDIR -lprotobuf
	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

s
