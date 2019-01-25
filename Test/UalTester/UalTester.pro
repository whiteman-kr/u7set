
QT  += core
QT  -= gui
QT  += network

TARGET = UalTester
CONFIG += console
CONFIG -= app_bundle


TEMPLATE = app

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet



# DESTDIR
#
win32 {
	CONFIG(debug, debug|release): DESTDIR = ../../bin/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin/release
}
unix {
	CONFIG(debug, debug|release): DESTDIR = ../../bin_unix/debug
	CONFIG(release, debug|release): DESTDIR = ../../bin_unix/release
}

SOURCES += \
        main.cpp \
    UalTester.cpp \
    ../../lib/CommandLineParser.cpp \
    ../../lib/CircularLogger.cpp \
    ../../lib/SimpleThread.cpp \
	../../lib/WUtils.cpp \
    ../../lib/HostAddressPort.cpp \
    ../../lib/SocketIO.cpp

HEADERS += \
	UalTester.h \
    ../../lib/CommandLineParser.h \
    ../../lib/OrderedHash.h \
    ../../lib/CircularLogger.h \
    ../../lib/SimpleThread.h \
	../../lib/WUtils.h \
    ../../lib/HostAddressPort.h \
    ../../lib/SocketIO.h
