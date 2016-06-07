#-------------------------------------------------
#
# Project created by QtCreator 2014-08-19T13:17:21
#
#-------------------------------------------------

QT      += core
QT      -= gui
QT      += network
QT	+= widgets
QT      += qml

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
# for creating version.h at first build
win32:system(IF NOT EXIST version.h (echo int VERSION_H = 0; > version.h))
unix:system([ -e ./version.h ] || touch ./version.h)
# for any build
versionTarget.target = version.h
versionTarget.depends = FORCE
win32 {
	contains(QMAKE_TARGET.arch, x86_64){
	    versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
	    qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
	    nmake & \
	    chdir $$PWD & \
	    $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/BaseService.pro
	}
	else{
	    versionTarget.commands = chdir $$PWD/../GetGitProjectVersion & \
	    qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
	    nmake & \
	    chdir $$PWD & \
	    $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/BaseService.pro
	}
}
unix {
    versionTarget.commands = cd $$PWD/../GetGitProjectVersion; \
	qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
	make; \
	cd $$PWD; \
	$$PWD/../bin_unix/GetGitProjectVersion $$PWD/BaseService.pro
}
PRE_TARGETDEPS += version.h
QMAKE_EXTRA_TARGETS += versionTarget


SOURCES += main.cpp \
    ../lib/UdpSocket.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/UdpFileTransfer.cpp \
    ../lib/Service.cpp \
    ../lib/SimpleThread.cpp

HEADERS += \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ../lib/CircularLogger.h \
	../lib/FscDataFormat.h \
    version.h \
    ../lib/JsonSerializable.h \
    ../lib/UdpFileTransfer.h \
    ../lib/Service.h \
    ../lib/SimpleThread.h

include(../qtservice/src/qtservice.pri)

win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

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
