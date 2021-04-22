#-------------------------------------------------
#
# Project created by QtCreator 2014-09-12T12:56:12
#
#-------------------------------------------------

QT       += core
QT       += gui
QT       += widgets
QT       += concurrent
QT       += serialport
QT       += network
QT       += sql
QT       += qml
QT       += xml
QT       += charts

#axcontainer

TARGET = Metrology
TEMPLATE = app

include(../qtpropertybrowser/src/qtpropertybrowser.pri)

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

#Application icon
#
win32:RC_ICONS += icons/Metrology.ico

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
	../lib/ModuleFirmware.cpp \
	../lib/AppSignalParam.cpp \
	../lib/BuildInfo.cpp \
	../lib/SoftwareSettings.cpp \
	../lib/DeviceHelper.cpp \
	../lib/OutputLog.cpp \
	../lib/Tuning/TuningSignalState.cpp \
	../lib/SoftwareInfo.cpp \
	../lib/TuningValue.cpp \
	../lib/AppSignal.cpp \
	../lib/SignalProperties.cpp \
	../lib/Ui/DialogAbout.cpp \
	../lib/ComparatorSet.cpp \
	../Builder/IssueLogger.cpp \
	DialogCalculator.cpp \
	DialogComparatorList.cpp \
	DialogList.cpp \
	DialogMeasurePoint.cpp \
	DialogMetrologyConnection.cpp \
	DialogObjectProperties.cpp \
	DialogOptions.cpp \
	DialogOptionsMvh.cpp \
	DialogRackList.cpp \
	DialogSignalList.cpp \
	DialogTuningSignalList.cpp \
	DialogTuningSourceList.cpp \
	MetrologyMain.cpp \
	MetrologyMainWindow.cpp \
	MetrologyConnection.cpp \
	MetrologySignal.cpp \
	UnitsConvertor.cpp \
	UnitsConvertorTable.cpp \
	Calibrator.cpp \
	CalibratorBase.cpp \
	MeasurePointBase.cpp \
	Options.cpp \
	PanelComparatorInfo.cpp \
	PanelFindMeasure.cpp \
	PanelSignalInfo.cpp \
	PanelStatistics.cpp \
	ProcessData.cpp \
	SelectSignalWidget.cpp \
	StatisticsBase.cpp \
	MeasureThread.cpp \
	CalibratorManager.cpp \
	MeasureViewHeader.cpp \
	MeasureView.cpp \
	Delegate.cpp \
	FolderPropertyManager.cpp \
	Database.cpp \
	SignalSocket.cpp \
	SignalBase.cpp \
	TuningSocket.cpp \
	TuningSignalBase.cpp \
	ConfigSocket.cpp \
	RackBase.cpp \
	MeasureBase.cpp \

HEADERS  += \
    Stable.h \
	../lib/AppSignal.h \
	../lib/CUtils.h \
	../lib/Factory.h \
	../lib/ModuleFirmware.h \
	../lib/Types.h \
	../lib/OrderedHash.h \
	../lib/PropertyObject.h \
	../lib/AppSignalParam.h \
	../lib/BuildInfo.h \
	../lib/SoftwareSettings.h \
	../lib/DeviceHelper.h \
	../lib/OutputLog.h \
	../lib/Tuning/TuningSignalState.h \
	../lib/SoftwareInfo.h \
	../lib/TuningValue.h \
	../lib/SignalProperties.h \
	../lib/Ui/DialogAbout.h \
	../lib/ComparatorSet.h \
	../Builder/IssueLogger.h \
	DialogCalculator.h \
	DialogComparatorList.h \
	DialogList.h \
	DialogMeasurePoint.h \
	DialogMetrologyConnection.h \
	DialogObjectProperties.h \
	DialogOptions.h \
	DialogOptionsMvh.h \
	DialogRackList.h \
	DialogSignalList.h \
	DialogTuningSignalList.h \
	DialogTuningSourceList.h \
	MetrologyMainWindow.h \
	Calibrator.h \
	CalibratorBase.h \
	MeasurePointBase.h \
	Options.h \
	MeasureThread.h \
	MetrologySignal.h \
	MetrologyConnection.h \
	CalibratorManager.h \
	MeasureViewHeader.h \
	MeasureView.h \
	Delegate.h \
	FolderPropertyManager.h \
	Database.h \
	PanelComparatorInfo.h \
	PanelFindMeasure.h \
	PanelSignalInfo.h \
	PanelStatistics.h \
	UnitsConvertor.h \
	UnitsConvertorTable.h \
	ProcessData.h \
	SelectSignalWidget.h \
	SignalSocket.h \
	SignalBase.h \
	StatisticsBase.h \
	TuningSocket.h \
	TuningSignalBase.h \
	ConfigSocket.h \
	RackBase.h \
	MeasureBase.h \


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

FORMS    +=

RESOURCES += \
    Resources.qrc

TRANSLATIONS = languages/Metrology_ru.ts \
    languages/Metrology_ru.qm

# Add curent dir to a list of library directory paths
#
unix:QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/./\''

# --
#
LIBS += -L$$DESTDIR
LIBS += -L.

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto

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

# HardwareLib
#
LIBS += -lHardwareLib

# DbLib !!!!!!!!!!!!!! REMOVE IN FUTURE, MUST IT BE HERE!?
#
LIBS += -lDbLib

# protobuf
#
LIBS += -lprotobuf
INCLUDEPATH += ./../Protobuf
