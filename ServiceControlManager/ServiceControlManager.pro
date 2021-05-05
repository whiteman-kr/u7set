#-------------------------------------------------
#
# Project created by QtCreator 2014-08-21T13:33:03
#
#-------------------------------------------------

QT       += core gui network qml xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scm
TEMPLATE = app

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

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
	../lib/SoftwareSettings.cpp \
    ../lib/DataSource.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
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
    ../lib/SoftwareSettings.h \
    ../lib/DataSource.h \
    ../lib/WidgetUtils.h \
    ../lib/OrderedHash.h \
    ../lib/Tuning/TuningSourceState.h \
    ../lib/Tuning/TuningSignalState.h \
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

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib

# ServiceLib
#
LIBS += -lServiceLib
win32:PRE_TARGETDEPS += $$DESTDIR/ServiceLib.lib

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
INCLUDEPATH += ./../Protobuf

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
