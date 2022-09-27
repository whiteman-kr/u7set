QT       += qml sql xml widgets concurrent gui printsupport

TARGET = Builder
TEMPLATE = lib
CONFIG += staticlib

include(../compiler.pri)
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


SOURCES += \
    ../lib/ClientBehavior.cpp \
    ../lib/DeviceHelper.cpp \
	../lib/LanControllerInfo.cpp \
    ../lib/LanControllerInfoHelper.cpp \
    ../lib/LogicModulesInfo.cpp \
    ../lib/OutputLog.cpp \
    ../lib/LogicModuleSet.cpp \
    ../lib/SoftwareSettings.cpp \
    ../lib/DataSource.cpp \
	../lib/SoftwareSettingsGetter.cpp \
    ../lib/Tuning/TuningFilter.cpp \
    ../lib/BuildInfo.cpp \
    ../lib/LmDescription.cpp \
    ../lib/Tuning/TuningSignalManager.cpp \
    ../lib/ConnectionsInfo.cpp \
	../lib/Tuning/TuningSignalState.cpp \
	../lib/ComparatorSet.cpp \
	../lib/TuningDataStorage.cpp \
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
    ReportGenerator.cpp \
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
	../lib/SoftwareSettingsGetter.h \
    ../lib/Tuning/TuningFilter.h \
    ../lib/BuildInfo.h \
    ../lib/LmDescription.h \
    ../lib/Tuning/TuningSignalManager.h \
    ../lib/ConnectionsInfo.h \
	../lib/Tuning/TuningSignalState.h \
	../lib/TuningDataStorage.h \
	../lib/ComparatorSet.h \
	../CommonLib/PropertyObject.h \
	../TuningService/TuningSource.h \
	../Metrology/MetrologySignal.h \
	../Metrology/MetrologyConnection.h \
	../Metrology/UnitsConvertor.h \
	../Metrology/UnitsConvertorTable.h \
	Builder.h \
	AppSignalProperties.h \
	AppSignalSetProvider.h \
    ConnectionStorage.h \
    Loopbacks.h \
    ReportGenerator.h \
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
    ../Test/CompilerTests/CompilerTests.js \
    ../Test/CompilerTests/InbusConversions.js


