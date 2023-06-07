#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T13:33:03
#
#-------------------------------------------------

QT       += core gui network qml xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scm
TEMPLATE = app

include(../compiler.pri)
include(../warnings.pri)
include(../sanitizer.pri)

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

SOURCES += \
    ../lib/DataSource.cpp \
    ../lib/WidgetUtils.cpp \
	../AppDataService/DynamicAppSignalState.cpp \
	../AppDataService/AppDataSource.cpp \
	../AppDataService/RtTrendsServer.cpp \
	MainWindow.cpp \
	ScanOptionsWidget.cpp \
	ScmMain.cpp \
	ScmTcpAppDataClient.cpp \
	ServiceTableModel.cpp \
	BaseServiceStateWidget.cpp \
	ConfigurationServiceWidget.cpp \
	TcpConfigServiceClient.cpp \
	AppDataServiceWidget.cpp \
	TuningServiceWidget.cpp \
	TcpTuningServiceClient.cpp \
	TuningSourceWidget.cpp \
	AppDataSourceWidget.cpp \

HEADERS  += \
	Stable.h \
    ../lib/DataSource.h \
    ../lib/WidgetUtils.h \
	../AppDataService/DynamicAppSignalState.h \
	../AppDataService/AppDataSource.h \
	../AppDataService/RtTrendsServer.h \
	MainWindow.h \
	AppDataServiceWidget.h \
	TuningServiceWidget.h \
	TcpTuningServiceClient.h \
	TuningSourceWidget.h \
	AppDataSourceWidget.h \
	TcpConfigServiceClient.h \
	ScanOptionsWidget.h \
	ScmTcpAppDataClient.h \
	ServiceTableModel.h \
	BaseServiceStateWidget.h \
	ConfigurationServiceWidget.h \

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

FORMS    +=

TRANSLATIONS = ./translations/ServiceControlManager_ru.ts \
               ./translations/ServiceControlManager_uk.ts

RESOURCES += \
    ServiceControlManager.qrc

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# ClientLib
#
LIBS += -lClientLib
win32:PRE_TARGETDEPS += $$DESTDIR/ClientLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libClientLib.a

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libOnlineLib.a

# ServiceLib
#
LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libServiceLib.a

# HardwareLib
#
LIBS += -lHardwareLib
win32:PRE_TARGETDEPS += $$DESTDIR/HardwareLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libHardwareLib.a

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

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libCommonLib.a

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
unix:PRE_TARGETDEPS += $$DESTDIR/libprotobuf.a
INCLUDEPATH += ./../Protobuf

