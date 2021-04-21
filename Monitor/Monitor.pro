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
    ../lib/Types.cpp \
    MonitorConfigController.cpp \
    TcpSignalClient.cpp \
    ../lib/AppSignal.cpp \
	../lib/AppSignalParam.cpp \
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
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Tuning/TuningSourceState.cpp \
    ../lib/Times.cpp \
    ../lib/SignalProperties.cpp \
    Trend/RtTrendTcpClient.cpp \
    Trend/ArchiveTrendTcpClient.cpp \
    MonitorTuningTcpClient.cpp \
    ../lib/LogFile.cpp \
    ../lib/Ui/DialogAlert.cpp \
    ../lib/Ui/UiTools.cpp \
    ../lib/Ui/DialogAbout.cpp \
    ../lib/Ui/DialogSourceInfo.cpp \
    TcpAppSourcesState.cpp \
    ../lib/Ui/TuningSourcesWidget.cpp \
    ../lib/Ui/AppDataSourcesWidget.cpp \
    DialogDataSources.cpp \
    ../lib/TcpClientsStatistics.cpp \
	../lib/ExportPrint.cpp

HEADERS  += \
    ../lib/ClientBehavior.h \
    ../lib/ComparatorSet.h \
    ../lib/ConstStrings.h \
    ../lib/ILogFile.h \
    ../lib/SoftwareSettings.h \
    ../lib/Ui/DialogSignalSnapshot.h \
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
    ../lib/Types.h \
    MonitorConfigController.h \
    TcpSignalClient.h \
    ../lib/AppSignal.h \
    ../lib/PropertyObject.h \
	../lib/AppSignalParam.h \
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
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/Tuning/TuningSourceState.h \
    ../lib/Times.h \
    ../lib/SignalProperties.h \
    Trend/RtTrendTcpClient.h \
    Trend/ArchiveTrendTcpClient.h \
    MonitorTuningTcpClient.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/Tuning/TuningSourceState.h \
    ../lib/LogFile.h \
    ../lib/Ui/DialogAlert.h \
    ../lib/Ui/UiTools.h \
    ../lib/Ui/DialogAbout.h \
    ../lib/Ui/DialogSourceInfo.h \
    TcpAppSourcesState.h \
    ../lib/Ui/TuningSourcesWidget.h \
    ../lib/Ui/AppDataSourcesWidget.h \
    DialogDataSources.h \
    ../lib/TcpClientsStatistics.h \
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
INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# TrendView library
#
LIBS += -lTrendView
INCLUDEPATH += $$PWD/../TrendView
DEPENDPATH += $$PWD/../TrendView

# Protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../Protobuf

# --
#

# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
    CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}

# OnlineLib
#
LIBS += -lOnlineLib

# UtilsLib
#
LIBS += -lUtilsLib


