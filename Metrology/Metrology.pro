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
gcc:CONFIG += c++20
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

#Application icon
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
    ../lib/Address16.cpp \
    ../lib/DbController.cpp \
    ../lib/DbProgress.cpp \
    ../lib/DbProgressDialog.cpp \
    ../lib/DbWorker.cpp \
    ../lib/MetrologyConnection.cpp \
    ../lib/SignalSetProvider.cpp \
    ../lib/ScriptDeviceObject.cpp \
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
    MainWindow.cpp \
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
    main.cpp \
    MeasureThread.cpp \
    CalibratorManager.cpp \
    MeasureViewHeader.cpp \
    MeasureView.cpp \
    Delegate.cpp \
    FolderPropertyManager.cpp \
    Database.cpp \
    Conversion.cpp \
    ../lib/Crc.cpp \
    ../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/ModuleFirmware.cpp \
    ../lib/SocketIO.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SimpleThread.cpp \
    SignalSocket.cpp \
    ../lib/Tcp.cpp \
    SignalBase.cpp \
    ../lib/AppSignal.cpp \
    TuningSocket.cpp \
    TuningSignalBase.cpp \
    ConfigSocket.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/TcpFileTransfer.cpp \
	../lib/SoftwareSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../Builder/ModulesRawData.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp \
    ../lib/MetrologySignal.cpp \
    RackBase.cpp \
    MeasureBase.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Signal.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Ui/DialogAbout.cpp \
	../lib/UnitsConvertor.cpp \
    ../lib/UnitsConvertorTable.cpp \
    ../lib/ComparatorSet.cpp

#../lib/ExcelHelper.cpp

HEADERS  += \
    ../lib/Address16.h \
    ../lib/DbController.h \
    ../lib/DbProgress.h \
    ../lib/DbProgressDialog.h \
    ../lib/DbWorker.h \
    ../lib/MetrologyConnection.h \
    ../lib/SignalSetProvider.h \
    ../lib/ScriptDeviceObject.h \
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
    MainWindow.h \
    Calibrator.h \
    CalibratorBase.h \
    MeasurePointBase.h \
    Options.h \
    MeasureThread.h \
    CalibratorManager.h \
    MeasureViewHeader.h \
    MeasureView.h \
    Delegate.h \
    FolderPropertyManager.h \
    Database.h \
    Conversion.h \
    PanelComparatorInfo.h \
    PanelFindMeasure.h \
    PanelSignalInfo.h \
    PanelStatistics.h \
    ProcessData.h \
    SelectSignalWidget.h \
    Stable.h \
    ../lib/Signal.h \
    ../lib/CUtils.h \
    ../lib/Crc.h \
    ../lib/Factory.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ModuleFirmware.h \
    ../lib/Types.h \
    ../lib/OrderedHash.h \
    ../lib/SocketIO.h \
    ../lib/PropertyObject.h \
    ../lib/XmlHelper.h \
    ../lib/HostAddressPort.h \
    ../lib/SimpleThread.h \
    SignalSocket.h \
    ../lib/Tcp.h \
    SignalBase.h \
    ../lib/AppSignal.h \
    StatisticsBase.h \
    TuningSocket.h \
    TuningSignalBase.h \
    ConfigSocket.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/TcpFileTransfer.h \
	../lib/SoftwareSettings.h \
    ../lib/DeviceHelper.h \
    ../Builder/ModulesRawData.h \
    ../Builder/IssueLogger.h \
    ../lib/OutputLog.h \
    ../lib/MetrologySignal.h \
    RackBase.h \
    MeasureBase.h \
    ../lib/CircularLogger.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../lib/SignalProperties.h \
    ../lib/Ui/DialogAbout.h \
	../lib/UnitsConvertor.h \
    ../lib/UnitsConvertorTable.h \
#../lib/ExcelHelper.h
    ../lib/ComparatorSet.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

FORMS    +=

RESOURCES += \
    Resources.qrc

TRANSLATIONS = languages/Metrology_ru.ts \
    languages/Metrology_ru.qm

#protobuf
#
LIBS += -L$$DESTDIR -lprotobuf
INCLUDEPATH += ./../Protobuf


DISTFILES += \
    ../Proto/network.proto \
	../Proto/serialization.proto


# Visual Leak Detector
#
win32 {
    CONFIG(debug, debug|release): LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/Win64"
	CONFIG(debug, debug|release): LIBS += -L"D:/Program Files (x86)/Visual Leak Detector/lib/Win64"
}
