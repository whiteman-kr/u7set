#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T14:26:26
#
#-------------------------------------------------

QT       += core gui widgets network xmlpatterns qml xml svg printsupport

TARGET = Monitor
TEMPLATE = app

INCLUDEPATH += $$PWD

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

# Optimization flags
#
win32 {
        CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -Od
        CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O2
}
unix {
        CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -O0
        CONFIG(release, debug|release): QMAKE_CXXFLAGS += -O3
}

#Application icon
win32:RC_ICONS += Images/Monitor.ico

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

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

SOURCES += main.cpp \
    ../lib/ClientBehavior.cpp \
    ../lib/ComparatorSet.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/Ui/DialogSignalSnapshot.cpp \
    ../lib/Ui/DialogTcpStatistics.cpp \
    ../lib/Ui/DragDropHelper.cpp \
    ../lib/Ui/SchemaListWidget.cpp \
    ../lib/Ui/TabWidgetEx.cpp \
    ../lib/Ui/TagSelectorWidget.cpp \
    MonitorMainWindow.cpp \
	MonitorCentralWidget.cpp \
    MonitorSignalInfo.cpp \
    MonitorSignalSnapshot.cpp \
	Settings.cpp \
    DialogSettings.cpp \
    ../lib/BuildInfo.cpp \
    MonitorSchemaWidget.cpp \
    MonitorConfigController.cpp \
    TcpSignalClient.cpp \
    ../lib/AppSignalManager.cpp \
    Statistics.cpp \
    ../lib/Ui/DialogSignalInfo.cpp \
    ../lib/Ui/DialogSignalSearch.cpp \
    DialogColumns.cpp \
    MonitorView.cpp \
    Trend/MonitorTrends.cpp \
    MonitorArchive.cpp \
    DialogChooseArchiveSignals.cpp \
    ArchiveTcpClient.cpp \
    ArchiveModelView.cpp \
    ArchiveData.cpp \
    TcpSignalRecents.cpp \
	MonitorSchemaManager.cpp \
    SelectSchemaWidget.cpp \    
	../lib/Tuning/TuningSignalManager.cpp \
    ../lib/Tuning/TuningTcpClient.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
#	../lib/AppSignalProperties.cpp \
    Trend/RtTrendTcpClient.cpp \
    Trend/ArchiveTrendTcpClient.cpp \
    MonitorTuningTcpClient.cpp \
    ../lib/Ui/DialogAlert.cpp \
    ../lib/Ui/DialogAbout.cpp \
    ../lib/Ui/DialogSourceInfo.cpp \
    TcpAppSourcesState.cpp \
    ../lib/Ui/TuningSourcesWidget.cpp \
    ../lib/Ui/AppDataSourcesWidget.cpp \
    DialogDataSources.cpp \
	../lib/ExportPrint.cpp

HEADERS  += \
    ../lib/ClientBehavior.h \
    ../lib/ComparatorSet.h \
    ../lib/ConstStrings.h \
	../UtilsLib/ILogFile.h \
    ../lib/SoftwareSettings.h \
    ../lib/Ui/DialogSignalSnapshot.h \
    ../lib/Ui/DialogTcpStatistics.h \
    ../lib/Ui/DragDropHelper.h \
    ../lib/Ui/SchemaListWidget.h \
    ../lib/Ui/TabWidgetEx.h \
    ../lib/Ui/TagSelectorWidget.h \
    MonitorMainWindow.h \
    MonitorCentralWidget.h \
    MonitorSignalInfo.h \
    MonitorSignalSnapshot.h \
	Stable.h \
	Settings.h \
    DialogSettings.h \
    ../lib/BuildInfo.h \
    MonitorSchemaWidget.h \
    MonitorConfigController.h \
    TcpSignalClient.h \
    ../CommonLib/PropertyObject.h \
    ../lib/AppSignalManager.h \
    Statistics.h \
    ../lib/Ui/DialogSignalInfo.h \
    ../lib/Ui/DialogSignalSearch.h \
    DialogColumns.h \
    MonitorView.h \
    Trend/MonitorTrends.h \
    MonitorArchive.h \
    DialogChooseArchiveSignals.h \
    ArchiveTcpClient.h \
    ArchiveModelView.h \
    ArchiveData.h \
    TcpSignalRecents.h \
    SelectSchemaWidget.h \
    MonitorSchemaManager.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../lib/Tuning/TuningTcpClient.h \
    ../lib/Tuning/TuningSourceState.h \
#	../lib/AppSignalProperties.h \
    Trend/RtTrendTcpClient.h \
    Trend/ArchiveTrendTcpClient.h \
    MonitorTuningTcpClient.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/Tuning/TuningSourceState.h \
    ../lib/Ui/DialogAlert.h \
    ../lib/Ui/DialogAbout.h \
    ../lib/Ui/DialogSourceInfo.h \
    TcpAppSourcesState.h \
    ../lib/Ui/TuningSourcesWidget.h \
    ../lib/Ui/AppDataSourcesWidget.h \
    DialogDataSources.h \
	../lib/ExportPrint.h

FORMS    += \
    DialogSettings.ui \
    ../lib/Ui/DialogSignalInfo.ui \
    DialogColumns.ui \
    DialogChooseArchiveSignals.ui

RESOURCES += \
    Monitor.qrc

DISTFILES += \
    Images/Logo.png \
    Images/NewSchema.svg \
    Images/Backward.svg \
    Images/Forward.svg \
    Images/ZoomIn.svg \
    Images/ZoomOut.svg \
    Images/Settings.svg \
    Images/Close.svg \
    Images/About.svg \
    Images/readme.txt \
    ../Proto/network.proto \
    ../Proto/serialization.proto \
    Images/Trends.svg \
    Images/Log.svg \
    Images/TuningSources.svg \
    Images/Monitor.ico


# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

# VFrame30 library
#
LIBS += -lVFrame30
win32:PRE_TARGETDEPS += $$DESTDIR/VFrame30.lib
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# TrendView library
#
LIBS += -lTrendView
win32:PRE_TARGETDEPS += $$DESTDIR/TrendView.lib
INCLUDEPATH += $$PWD/../TrendView
DEPENDPATH += $$PWD/../TrendView

# Protobuf
#
LIBS += -lprotobuf
win32:PRE_TARGETDEPS += $$DESTDIR/protobuf.lib
INCLUDEPATH += ./../Protobuf

# OnlineLib
#
LIBS += -lOnlineLib
win32:PRE_TARGETDEPS += $$DESTDIR/OnlineLib.lib

# UtilsLib
#
LIBS += -lUtilsLib
win32:PRE_TARGETDEPS += $$DESTDIR/UtilsLib.lib

# CommonLib
#
LIBS += -lCommonLib
win32:PRE_TARGETDEPS += $$DESTDIR/CommonLib.lib

# AppSignalLib
#
LIBS += -lAppSignalLib
win32:PRE_TARGETDEPS += $$DESTDIR/AppSignalLib.lib

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
