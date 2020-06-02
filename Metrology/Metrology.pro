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
#axcontainer

TARGET = Metrology
TEMPLATE = app

include(../qtpropertybrowser/src/qtpropertybrowser.pri)

#c++17 support
#
gcc:CONFIG += c++1z
win32:QMAKE_CXXFLAGS += /std:c++17		#CONFIG += c++17 has no effect yet

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
    ../lib/MemLeaksDetection.cpp \
    ../lib/SignalMacro.cpp \
    MainWindow.cpp \
    Calibrator.cpp \
    CalibratorBase.cpp \
    OptionsDialog.cpp \
    Options.cpp \
    OptionsPointsDialog.cpp \
    main.cpp \
    MeasureThread.cpp \
    CalibratorManager.cpp \
    MeasureViewHeader.cpp \
    MeasureView.cpp \
    OptionsMvhDialog.cpp \
    Delegate.cpp \
    FolderPropertyManager.cpp \
    Database.cpp \
    Conversion.cpp \
    Calculator.cpp \
    ../lib/Crc.cpp \
    ../lib/DbStruct.cpp \
    ../lib/DeviceObject.cpp \
    ../lib/ModuleFirmware.cpp \
    ../lib/ProtoSerialization.cpp \
    ../lib/SocketIO.cpp \
    ../lib/XmlHelper.cpp \
    ../lib/HostAddressPort.cpp \
    ../lib/SimpleThread.cpp \
    SignalSocket.cpp \
    ../lib/Tcp.cpp \
    SignalBase.cpp \
    ../Proto/network.pb.cc \
    ../Proto/serialization.pb.cc \
    ../lib/AppSignal.cpp \
    SignalList.cpp \
    FindMeasurePanel.cpp \
    SignalInfoPanel.cpp \
    ExportData.cpp \
    FindData.cpp \
    TuningSocket.cpp \
    TuningSignalBase.cpp \
    TuningSignalList.cpp \
    ConfigSocket.cpp \
    ../lib/CfgServerLoader.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/TcpFileTransfer.cpp \
    ../lib/ServiceSettings.cpp \
    ../lib/DeviceHelper.cpp \
    ../Builder/ModulesRawData.cpp \
    ../Builder/IssueLogger.cpp \
    ../lib/OutputLog.cpp \
    ../lib/MetrologySignal.cpp \
    RackList.cpp \
    ObjectProperties.cpp \
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
    StatisticBase.cpp \
    StatisticPanel.cpp \
    ../lib/ComparatorSet.cpp \
    ComparatorList.cpp \
    ComparatorInfoPanel.cpp \
    SignalConnectionList.cpp \
    SignalConnectionBase.cpp

#../lib/ExcelHelper.cpp

HEADERS  += \
    ../lib/Address16.h \
	../lib/MemLeaksDetection.h \
    ../lib/SignalMacro.h \
    MainWindow.h \
    Calibrator.h \
    CalibratorBase.h \
    OptionsDialog.h \
    Options.h \
    OptionsPointsDialog.h \
    MeasureThread.h \
    CalibratorManager.h \
    MeasureViewHeader.h \
    MeasureView.h \
    OptionsMvhDialog.h \
    Delegate.h \
    FolderPropertyManager.h \
    Database.h \
    Conversion.h \
    Calculator.h \
    Stable.h \
    ObjectVector.h \
    ../lib/Signal.h \
    ../lib/CUtils.h \
    ../lib/Crc.h \
    ../lib/Factory.h \
    ../lib/DeviceObject.h \
    ../lib/DbStruct.h \
    ../lib/ModuleFirmware.h \
    ../lib/ProtoSerialization.h \
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
    ../Proto/network.pb.h \
    ../Proto/serialization.pb.h \
    ../lib/AppSignal.h \
    SignalList.h \
    FindMeasurePanel.h \
    SignalInfoPanel.h \
    ExportData.h \
    FindData.h \
    TuningSocket.h \
    TuningSignalBase.h \
    TuningSignalList.h \
    ConfigSocket.h \
    ../lib/CfgServerLoader.h \
    ../lib/BuildInfo.h \
    ../lib/TcpFileTransfer.h \
    ../lib/ServiceSettings.h \
    ../lib/DeviceHelper.h \
    ../Builder/ModulesRawData.h \
    ../Builder/IssueLogger.h \
    ../lib/OutputLog.h \
    ../lib/MetrologySignal.h \
    RackList.h \
    ObjectProperties.h \
    RackBase.h \
    MeasureBase.h \
    ../lib/CircularLogger.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../Builder/CfgFiles.h \
    ../lib/SignalProperties.h \
    ../lib/Ui/DialogAbout.h \
	../lib/UnitsConvertor.h \
    ../lib/UnitsConvertorTable.h \
#../lib/ExcelHelper.h
    StatisticBase.h \
    StatisticPanel.h \
    ../lib/ComparatorSet.h \
    ComparatorList.h \
    ComparatorInfoPanel.h \
    SignalConnectionList.h \
    SignalConnectionBase.h

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

FORMS    +=

RESOURCES += \
    Resources.qrc

TRANSLATIONS = translations/Metrology_ru.ts \
		translations/Metrology_uk.ts

OTHER_FILES += \
    translations/Metrology_ru.ts \
    translations/Metrology_uk.ts

# Q_DEBUG define
#
CONFIG(debug, debug|release): DEFINES += Q_DEBUG

# _DEBUG define, Windows memmory detection leak depends on it
#
CONFIG(debug, debug|release): DEFINES += _DEBUG


# Remove Protobuf 4996 warning, Can't remove it in sources, don't know why
#
win32:QMAKE_CXXFLAGS += -D_SCL_SECURE_NO_WARNINGS

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

#protobuf
#
win32 {
		LIBS += -L$$DESTDIR -lprotobuf

		INCLUDEPATH += ./../Protobuf
}
unix {
		LIBS += -lprotobuf
}

DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto
