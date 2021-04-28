QT       += qml sql xml widgets concurrent

TARGET = Builder
TEMPLATE = lib
CONFIG += staticlib

# c++20 support
#
unix:QMAKE_CXXFLAGS += --std=c++20			# CONFIG += c++20 has no effect yet
win32:QMAKE_CXXFLAGS += /std:c++latest

include(../warnings.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = Stable.h

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

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DEFINES += IS_BUILDER

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../lib/ClientBehavior.cpp \
    ../lib/DeviceHelper.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/LogicModulesInfo.cpp \
    ../lib/OutputLog.cpp \
    ../lib/LogicModuleSet.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/DataSource.cpp \
    ../lib/CsvFile.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/LmDescription.cpp \
    ../lib/Tuning/TuningSignalManager.cpp \
    ../lib/ConnectionsInfo.cpp \
	../lib/Tuning/TuningSignalState.cpp \
	../lib/Times.cpp \
	../lib/ComparatorSet.cpp \
	../TuningService/TuningDataStorage.cpp \
	../TuningService/TuningSource.cpp \
	../Metrology/MetrologySignal.cpp \
	../Metrology/MetrologyConnection.cpp \
	../Metrology/UnitsConvertor.cpp \
	../Metrology/UnitsConvertorTable.cpp \
    Builder.cpp \
	AppSignalProperties.cpp \
	AppSignalSetProvider.cpp \
    ConnectionStorage.cpp \
    IssueLogger.cpp \
	DbMetrologyConnection.cpp \
    BuildWorkerThread.cpp \
    BuildResultWriter.cpp \
    Loopbacks.cpp \
    ModuleFirmwareWriter.cpp \
    Parser.cpp \
    SignalSet.cpp \
    Busses.cpp \
    OptoModule.cpp \
	RawDataDescription.cpp \
    SignalsHeap.cpp \
    SubsystemStorage.cpp \
    UalItems.cpp \
    ApplicationLogicCode.cpp \
    LmMemoryMap.cpp \
    ConfigurationBuilder.cpp \
    ApplicationLogicCompiler.cpp \
    ModuleLogicCompiler.cpp \
    MemWriteMap.cpp \
    SoftwareCfgGenerator.cpp \
    AppDataServiceCfgGenerator.cpp \
    ModulesRawData.cpp \
    DiagDataServiceCfgGenerator.cpp \
    MonitorCfgGenerator.cpp \
    TuningClientCfgGenerator.cpp \
    TuningServiceCfgGenerator.cpp \
    ConfigurationServiceCfgGenerator.cpp \
    ArchivingServiceCfgGenerator.cpp \
    MetrologyCfgGenerator.cpp \
    LmDescriptionSet.cpp \
    BdfFile.cpp \
    FbParamCalculation.cpp \
    TuningBuilder.cpp \
    RunOrder.cpp \
    TestClientCfgGenerator.cpp \
    Context.cpp \

HEADERS += \
    ../lib/ClientBehavior.h \
    ../lib/ConstStrings.h \
    ../lib/DeviceHelper.h \
    ../lib/LanControllerInfo.h \
    ../lib/LanControllerInfoHelper.h \
    ../lib/LogicModulesInfo.h \
    ../lib/OutputLog.h \
    ../lib/LogicModuleSet.h \
    ../lib/SoftwareSettings.h \
    ../lib/DataSource.h \
    ../lib/CsvFile.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/BuildInfo.h \
    ../lib/LmDescription.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../lib/ConnectionsInfo.h \
	../lib/Tuning/TuningSignalState.h \
	../lib/Times.h \
	../lib/ComparatorSet.h \
	../CommonLib/PropertyObject.h \
	../TuningService/TuningSource.h \
	../TuningService/TuningDataStorage.h \
	../Metrology/MetrologySignal.h \
	../Metrology/MetrologyConnection.h \
	../Metrology/UnitsConvertor.h \
	../Metrology/UnitsConvertorTable.h \
	Builder.h \
	AppSignalProperties.h \
	AppSignalSetProvider.h \
    ConnectionStorage.h \
    Loopbacks.h \
    SignalsHeap.h \
    Stable.h \
    IssueLogger.h \
	DbMetrologyConnection.h \
	BuildWorkerThread.h \
    BuildResultWriter.h \
    ModuleFirmwareWriter.h \
    Parser.h \
    SignalSet.h \
    Busses.h \
    OptoModule.h \
    RawDataDescription.h \
    SubsystemStorage.h \
    UalItems.h \
    ApplicationLogicCode.h \
    LmMemoryMap.h \
    TuningBuilder.h \
    ConfigurationBuilder.h \
    ApplicationLogicCompiler.h \
    ModuleLogicCompiler.h \
    MemWriteMap.h \
    SoftwareCfgGenerator.h \
    AppDataServiceCfgGenerator.h \
    ModulesRawData.h \
    DiagDataServiceCfgGenerator.h \
    MonitorCfgGenerator.h \
    TuningClientCfgGenerator.h \
    TuningServiceCfgGenerator.h \
    ConfigurationServiceCfgGenerator.h \
    ArchivingServiceCfgGenerator.h \
    MetrologyCfgGenerator.h \
    LmDescriptionSet.h \
    BdfFile.h \
    RunOrder.h \
    TestClientCfgGenerator.h \
    Context.h \

unix {
    target.path = /usr/lib
    INSTALLS += target
}

# protobuf
#
INCLUDEPATH += ./../Protobuf


DISTFILES += \
    ../Proto/network.proto \
    ../Proto/serialization.proto \
    ../Test/CompilerTests/CompilerTests.js


