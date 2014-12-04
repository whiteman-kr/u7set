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
    ../Proto/proto_compile.bat \
    ../Proto/serialization.proto \
    ../Proto/proto_compile.sh

HEADERS += VFrame30Lib_global.h \
    Stable.h \
    VideoItem.h \
    VideoItemSignal.h \
    VideoItemRect.h \
    VideoItemLink.h \
    VideoItemLine.h \
    VideoItemFblElement.h \
    VideoItemConnectionLine.h \
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
    Print.h \
    VFrame30Library.h \
    HorzVertLinks.h \
	../include/TypesAndEnums.h \
    VideoFrameManager.h \
    VideoFrameWidget.h \
    VideoFrameWidgetAgent.h \
    VideoFrameAgent.h \
    FrameHistoryItem.h \
	Configuration.h \
	DebugInstCounter.h \
    VFrame30.h \
    ../include/StreamedData.h \
    ../include/ProtoSerialization.h \
    ../include/CUtils.h \
    SchemeLayer.h \
    Scheme.h \
    WorkflowScheme.h \
    LogicScheme.h \
    DiagScheme.h \
    WiringScheme.h \
    SchemeView.h

SOURCES += \
    VideoItem.cpp \
    VideoItemSignal.cpp \
    VideoItemRect.cpp \
    VideoItemLink.cpp \
    VideoItemLine.cpp \
    VideoItemFblElement.cpp \
    VideoItemConnectionLine.cpp \
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
    Print.cpp \
    Stable.cpp \
    VFrame30Library.cpp \
    HorzVertLinks.cpp \
    VideoFrameManager.cpp \
    VideoFrameWidget.cpp \
    VideoFrameWidgetAgent.cpp \
    VideoFrameAgent.cpp \
    FrameHistoryItem.cpp \
	Configuration.cpp \
    ../lib/StreamedData.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/CUtils.cpp \
    SchemeLayer.cpp \
    Scheme.cpp \
    WorkflowScheme.cpp \
    LogicScheme.cpp \
    DiagScheme.cpp \
    WiringScheme.cpp \
    SchemeView.cpp

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

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

# Protobuf
#
#!include(protobuf.pri) {
#	error("Couldn't find the protobuf.pri file!")
#}

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

