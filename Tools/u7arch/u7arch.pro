QT  += core
QT  -= gui
QT  += network

TARGET = u7arch
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include(../../compiler.pri)
include(../../warnings.pri)
include(../../sanitizer.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


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
	../../ArchivingService/ArchFileRecord.cpp \
	ArchUtils.cpp \
	main.cpp \

HEADERS += \
	../../ArchivingService/ArchFileRecord.h \
	Stable.h \
	ArchUtils.h \

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

INCLUDEPATH += ./../../Protobuf

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

CONFIG(release, debug|release): unix:QMAKE_CXXFLAGS += -DNDEBUG

# UtilsLib
#
#LIBS += -lUtilsLib

# AppSignalLib
#
#LIBS += -lAppSignalLib

# CommonLib
#
#LIBS += -lCommonLib
