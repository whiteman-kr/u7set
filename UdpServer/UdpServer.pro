#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T16:14:34
#
#-------------------------------------------------

QT       += core gui qml sql xml

QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpServer
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

## Force prebuild version control info
##
#win32 {
#        contains(QMAKE_TARGET.arch, x86_64){
#            QMAKE_CLEAN += $$PWD/../bin_Win64/GetGitProjectVersion.exe
#            system(IF NOT EXIST $$PWD/../bin_Win64/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
#            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#            nmake))
#            system(chdir $$PWD & \
#            $$PWD/../bin_Win64/GetGitProjectVersion.exe $$PWD/UdpServer.pro)
#        }
#        else{
#            QMAKE_CLEAN += $$PWD/../bin_Win32/GetGitProjectVersion.exe
#            system(IF NOT EXIST $$PWD/../bin_Win32/GetGitProjectVersion.exe (chdir $$PWD/../GetGitProjectVersion & \
#            qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\" & \
#            nmake))
#            system(chdir $$PWD & \
#            $$PWD/../bin_Win32/GetGitProjectVersion.exe $$PWD/UdpServer.pro)
#        }
#}
#unix {
#    QMAKE_CLEAN += $$PWD/../bin_unix/GetGitProjectVersion
#    system(cd $$PWD/../GetGitProjectVersion; \
#        qmake \"OBJECTS_DIR = $$OUT_PWD/../GetGitProjectVersion/release\"; \
#        make; \
#        cd $$PWD; \
#        $$PWD/../bin_unix/GetGitProjectVersion $$PWD/UdpServer.pro)
#}


SOURCES +=\
    ServerMain.cpp \
    ../lib/UdpSocket.cpp \
    ServerSocket.cpp \
	../lib/Service.cpp \
    ServerMainWindow.cpp \
    ../lib/SocketIO.cpp \
    ../lib/CircularLogger.cpp \
	../lib/ProtoUdp.cpp \
	../lib/Tcp.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/TcpFileTransfer.cpp \
	../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/JsonSerializable.cpp \
    ../lib/HostAddressPort.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/CommandLineParser.cpp

HEADERS  += \
    ../lib/SocketIO.h \
    ../lib/UdpSocket.h \
    ServerSocket.h \
	../lib/Service.h \
    ServerMainWindow.h \
    ../lib/CircularLogger.h \
    version.h \
	../lib/ProtoUdp.h \
    ../lib/Factory.h \
	../lib/Tcp.h \
    ../lib/SimpleThread.h \
    ../lib/TcpFileTransfer.h \
	../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/JsonSerializable.h \
    ../lib/HostAddressPort.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/CommandLineParser.h


include(../qtservice/src/qtservice.pri)

FORMS    += \
    ServerMainWindow.ui


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
