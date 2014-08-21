#-------------------------------------------------
#
# Project created by QtCreator 2012-10-02T18:35:47
#
#-------------------------------------------------
TARGET = VFrame30

TEMPLATE = lib
QT += script widgets

win32:LIBS += -lGdi32

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


OTHER_FILES += \
	VideoFrame.proto

HEADERS += VFrame30Lib_global.h \
    Stable.h \
    VideoLayer.h \
    VideoItem.h \
    VFrame30.pb.h \
    VideoItemSignal.h \
    VideoItemRect.h \
    VideoItemLink.h \
    VideoItemLine.h \
    VideoItemFblElement.h \
    VideoItemConnectionLine.h \
    VideoFrameWiring.h \
    VideoFrameTech.h \
    VideoFrameLogic.h \
    VideoFrameDiag.h \
    Settings.h \
    PosRectImpl.h \
    PosLineImpl.h \
    PosConnectionImpl.h \
    FontParam.h \
    FblItemRect.h \
    FblItemLine.h \
    FblItem.h \
    FblConnectionsImpl.h \
    Fbl.h \
    DrawParam.h \
    VideoFrame.h \
    Print.h \
    VFrame30Library.h \
    Factory.h \
    HorzVertLinks.h \
	../include/TypesAndEnums.h \
	../include/VFrameUtils.h \
    VideoFrameManager.h \
    VideoFrameView.h \
    VideoFrameWidget.h \
    VideoFrameWidgetAgent.h \
    VideoFrameAgent.h \
    FrameHistoryItem.h \
	Configuration.h \
	DebugInstCounter.h \
    VFrame30.h

SOURCES += VideoLayer.cpp \
    VideoItem.cpp \
    VFrame30.pb.cpp \
    VideoItemSignal.cpp \
    VideoItemRect.cpp \
    VideoItemLink.cpp \
    VideoItemLine.cpp \
    VideoItemFblElement.cpp \
    VideoItemConnectionLine.cpp \
    VideoFrameWiring.cpp \
    VideoFrameTech.cpp \
    VideoFrameLogic.cpp \
    VideoFrameDiag.cpp \
    Settings.cpp \
    PosRectImpl.cpp \
    PosLineImpl.cpp \
    PosConnectionImpl.cpp \
    FontParam.cpp \
    FblItemRect.cpp \
    FblItemLine.cpp \
    FblItem.cpp \
    Fbl.cpp \
    DrawParam.cpp \
    VideoFrame.cpp \
    Print.cpp \
    Stable.cpp \
    VFrame30Library.cpp \
    HorzVertLinks.cpp \
	../lib/VFrameUtils.cpp \
    VideoFrameManager.cpp \
    VideoFrameView.cpp \
    VideoFrameWidget.cpp \
    VideoFrameWidgetAgent.cpp \
    VideoFrameAgent.cpp \
    FrameHistoryItem.cpp \
	Configuration.cpp

DEFINES += VFRAME30LIB_LIBRARY
#CONFIG(debug, debug|release): DEFINES += DEBUG

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

#c++11 support for GCC
#
unix:QMAKE_CXXFLAGS += -std=c++11

#Optimization flags
#
win32 {
}
unix {
	CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
	CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

##protobuf
##
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

# Protobuf
#
!include(protobuf.pri) {
	error("Couldn't find the protobuf.pri file!")
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
