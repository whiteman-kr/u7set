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


CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h


SOURCES += \
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
    Statistic.cpp \
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
    OutputSignalList.cpp \
    ObjectProperties.cpp \
    OutputSignalBase.cpp \
    RackBase.cpp \
    MeasureBase.cpp \
    ../lib/CircularLogger.cpp \
    ../lib/Tuning/TuningSignalState.cpp \
    ../lib/SoftwareInfo.cpp \
    ../lib/TuningValue.cpp \
    ../lib/Signal.cpp \
    ../lib/PropertyObject.cpp \
    ../lib/SignalProperties.cpp \
    ../lib/Ui/DialogAbout.cpp \
	../lib/UnitsConvertor.cpp \
    ../lib/UnitsConvertorTable.cpp

#../lib/ExcelHelper.cpp

HEADERS  += \
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
    Statistic.h \
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
    OutputSignalList.h \
    ObjectProperties.h \
    OutputSignalBase.h \
    RackBase.h \
    MeasureBase.h \
    ../lib/CircularLogger.h \
    ../lib/Tuning/TuningSignalState.h \
    ../lib/SoftwareInfo.h \
    ../lib/TuningValue.h \
    ../Builder/CfgFiles.h \
    ../lib/PropertyObject.h \
    ../lib/SignalProperties.h \
    ../lib/Ui/DialogAbout.h \
	../lib/UnitsConvertor.h \
    ../lib/UnitsConvertorTable.h
#../lib/ExcelHelper.h

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
