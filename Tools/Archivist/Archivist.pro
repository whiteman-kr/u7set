#-------------------------------------------------
#
# Project created by QtCreator 2014-11-21T11:49:17
#
#-------------------------------------------------

QT += core
QT += network
QT += widgets
QT -= gui
QT += xml

TARGET = Archivist
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../../compiler.pri)
include(../../warnings.pri)
include(../../sanitizer.pri)
include(../../codecoverage.pri)

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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

HEADERS += \
    Archivist.h \
    DbArchivist.h \
    FileArchivist.h \
	Stable.h \

SOURCES += \
    Archivist.cpp \
	ArchivistMain.cpp \
    DbArchivist.cpp \
    FileArchivist.cpp

INCLUDEPATH += ./../../Protobuf

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

#RESOURCES += \
#    Database/Database.qrc

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# AppSignalLib
#
#LIBS += -lAppSignalLib
#win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib
#unix:PRE_TARGETDEPS += $$DESTDIR/libAppSignalLib.a
# ServiceLib
#
LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libServiceLib.a

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libUtilsLib.a

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

