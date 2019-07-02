#-------------------------------------------------
#
# Project created by QtCreator 2015-05-29T14:26:26
#
#-------------------------------------------------

QT       += core gui widgets network xmlpatterns qml xml svg printsupport

TARGET = Monitor
TEMPLATE = app

INCLUDEPATH += $$PWD

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

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
    MonitorMainWindow.cpp \
	MonitorCentralWidget.cpp \
	Settings.cpp \
    ../lib/SocketIO.cpp \
    DialogSettings.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/Tcp.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/SimpleThread.cpp \
    MonitorSchemaWidget.cpp \
    ../lib/Types.cpp \
    MonitorConfigController.cpp \
    ../Proto/network.pb.cc \
    TcpSignalClient.cpp \
    ../Proto/serialization.pb.cc \
    ../lib/Signal.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/DbStruct.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/AppSignal.cpp \
    ../lib/AppSignalManager.cpp \
    Statistics.cpp \
    DialogSignalInfo.cpp \
    DialogSignalSearch.cpp \
    DialogSignalSnapshot.cpp \
    DialogColumns.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/CircularLogger.cpp \
    MonitorView.cpp \
    Trend/MonitorTrends.cpp \
    Trend/DialogChooseTrendSignals.cpp \
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
    MonitorMainWindow.h \
    MonitorCentralWidget.h \
	Stable.h \
	Settings.h \
    ../lib/SocketIO.h \
    DialogSettings.h \
    ../lib/Tcp.h \
    ../lib/TcpFileTransfer.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/SimpleThread.h \
    MonitorSchemaWidget.h \
    ../lib/Types.h \
    MonitorConfigController.h \
    ../Proto/network.pb.h \
    TcpSignalClient.h \
    ../lib/Hash.h \
    ../Proto/serialization.pb.h \
    ../lib/Signal.h \
    ../lib/PropertyObject.h \
    ../lib/XmlHelper.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ProtoSerialization.h \
    ../lib/AppSignal.h \
    ../lib/AppSignalManager.h \
    Statistics.h \
    DialogSignalInfo.h \
    DialogSignalSearch.h \
    DialogSignalSnapshot.h \
    DialogColumns.h \
    ../lib/HostAddressPort.h \
    ../lib/CircularLogger.h \
    MonitorView.h \
    Trend/MonitorTrends.h \
    Trend/DialogChooseTrendSignals.h \
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
    DialogSignalInfo.ui \
    DialogSignalSearch.ui \
    DialogSignalSnapshot.ui \
    DialogColumns.ui \
    Trend/DialogChooseTrendSignals.ui \
    DialogChooseArchiveSignals.ui


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


# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# _DEBUG define, Windows memmory detection leak depends on it
#
CONFIG(debug, debug|release): DEFINES += _DEBUG

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''


# VFrame30 library
# $unix:!macx|win32: LIBS += -L$$OUT_PWD/../VFrame30/ -lVFrame30
#
win32 {
	CONFIG(debug, debug|release): LIBS += -L../bin/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin/release/ -lVFrame30
}
unix {
	CONFIG(debug, debug|release): LIBS += -L../bin_unix/debug/ -lVFrame30
	CONFIG(release, debug|release): LIBS += -L../bin_unix/release/ -lVFrame30
}

INCLUDEPATH += ../VFrame30
DEPENDPATH += ../VFrame30

# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

#protobuf
#
win32 {
	LIBS += -L$$DESTDIR -lprotobuf

	INCLUDEPATH += ./../Protobuf
}
unix {
	LIBS += -lprotobuf
}

RESOURCES += \
    Monitor.qrc

DISTFILES += \
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

# TrendView library
#
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../TrendView/release/ -lTrendView
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../TrendView/debug/ -lTrendView
else:unix:!macx: LIBS += -L$$OUT_PWD/../TrendView/ -lTrendView

INCLUDEPATH += $$PWD/../TrendView
DEPENDPATH += $$PWD/../TrendView

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/release/libTrendView.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/debug/libTrendView.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/release/TrendView.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../TrendView/debug/TrendView.lib
else:unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../TrendView/libTrendView.a
