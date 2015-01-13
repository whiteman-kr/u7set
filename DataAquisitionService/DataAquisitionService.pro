#-------------------------------------------------
#
# Project created by QtCreator 2014-11-21T11:50:41
#
#-------------------------------------------------

QT       += core

QT       -= gui

QT       += network

QT		 += widgets

TARGET = DataSrv
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app



SOURCES += main.cpp \
	../lib/UdpSocket.cpp \
	../lib/BaseService.cpp \
	../lib/SocketIO.cpp \
	../lib/CircularLogger.cpp \
    DataAquisitionService.cpp \
    ../lib/DataSource.cpp

HEADERS += \
	../include/SocketIO.h \
	../include/UdpSocket.h \
	../include/BaseService.h \
	../include/CircularLogger.h \
    DataAquisitionService.h \
    ../include/DataSource.h

include(../qtservice/src/qtservice.pri)

unix:QMAKE_CXXFLAGS += -std=c++11

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
