#-------------------------------------------------
#
# Project created by QtCreator 2015-10-29T13:51:00
#
#-------------------------------------------------

QT += core gui
QT += network
QT += testlib qml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpClient
TEMPLATE = app


SOURCES +=\
	TcpClientMainWindow.cpp \
    TcpClientMain.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/Tcp.cpp \
    ../lib/SocketIO.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../AppDataService/TcpAppDataClient.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc

HEADERS  += TcpClientMainWindow.h \
    ../include/SimpleThread.h \
    ../include/Tcp.h \
    ../include/SocketIO.h \
    ../include/TcpFileTransfer.h \
    ../include/CfgServerLoader.h \
    ../include/BuildInfo.h \
    ../include/Utils.h \
    ../include/Md5Hash.h \
    ../AppDataService/TcpAppDataClient.h \
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h

FORMS    += TcpClientMainWindow.ui


unix:QMAKE_CXXFLAGS += -std=c++11

CONFIG(debug, debug|release): DEFINES += Q_DEBUG

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
