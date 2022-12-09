#-------------------------------------------------
#
# Project created by QtCreator 2015-06-24T11:50:49
#
#-------------------------------------------------

QT       += core gui network qml xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PacketViewer
TEMPLATE = app

include(../../compiler.pri)
include(../../warnings.pri)
include(../../sanitizer.pri)

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
	../../lib/DataSource.cpp \
	../../lib/LanControllerInfo.cpp \
	main.cpp \
	SourceListWidget.cpp \
	PacketSourceModel.cpp \
	SourceStatusWidget.cpp \
	PacketBufferTableModel.cpp \
	SignalTableModel.cpp \
	SendTuningFrameWidget.cpp \


HEADERS  += \
	Stable.h \
	../../Proto/serialization.pb.h \
	../../lib/DataSource.h \
	../../lib/LanControllerInfo.h \
	SourceListWidget.h \
	PacketSourceModel.h \
	SourceStatusWidget.h \
	PacketBufferTableModel.h \
	SignalTableModel.h \
	SendTuningFrameWidget.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# HardwareLib
#
LIBS += -lHardwareLib
win32:PRE_TARGETDEPS += $$DESTDIR/HardwareLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libHardwareLib.a

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../../Protobuf

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

