#-------------------------------------------------
#
# Project created by QtCreator 2017-05-24T10:51:11
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += serialport
QT       += widgets

TARGET = CommView
TEMPLATE = app

# c++20 support
#
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../../warnings.pri)

#Application icon
win32:RC_ICONS += icons/CommView.ico

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

SOURCES += main.cpp\
        MainWindow.cpp \
    Options.cpp \
    SerialPortDialog.cpp \
    SerialPortList.cpp \
    SerialPortWorker.cpp \
	../../lib/Crc.cpp \
    WorkerBase.cpp \
    TestResultDialog.cpp \
	OptionDialog.cpp

HEADERS  += MainWindow.h \
    Options.h \
    SerialPortDialog.h \
    SerialPortList.h \
    SerialPortWorker.h \
    SerialPortPacket.h \
	../../lib/Crc.h \
    WorkerBase.h \
    TestResultDialog.h \
    OptionDialog.h \
	Stable.h

RESOURCES += \
	resources.qrc

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

DISTFILES +=


# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
