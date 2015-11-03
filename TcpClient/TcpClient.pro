#-------------------------------------------------
#
# Project created by QtCreator 2015-10-29T13:51:00
#
#-------------------------------------------------

QT += core gui
QT += network
QT += testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpClient
TEMPLATE = app


SOURCES +=\
        TcpClientMainWindow.cpp \
    TcpClientMain.cpp \
    ../lib/SimpleThread.cpp \
    ../lib/Tcp.cpp \
    ../lib/SocketIO.cpp \
    ../lib/TcpFileTransfer.cpp

HEADERS  += TcpClientMainWindow.h \
    ../include/SimpleThread.h \
    ../include/Tcp.h \
    ../include/SocketIO.h \
    ../include/TcpFileTransfer.h

FORMS    += TcpClientMainWindow.ui


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
