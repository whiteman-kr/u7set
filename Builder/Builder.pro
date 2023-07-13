QT       += qml sql xml widgets concurrent gui printsupport

TARGET = Builder
TEMPLATE = lib
CONFIG += staticlib

include(../compiler.pri)
include(../warnings.pri)
include(../codecoverage.pri)

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
	AppLogicCode.cpp \
	AppLogicCompiler.cpp \
	CodeItem.cpp \
	DeviceHelper.cpp \
	LanControllerInfoHelper.cpp \
    ../lib/LogicModuleSet.cpp \
    ../lib/DataSource.cpp \
    ../lib/Tuning/TuningFilter.cpp \
	../lib/TuningDataStorage.cpp \
	../TuningService/TuningSource.cpp \
	../Metrology/MetrologySignal.cpp \
	../Metrology/MetrologyConnection.cpp \
	../Metrology/UnitsConvertor.cpp \
	../Metrology/UnitsConvertorTable.cpp \
	../GatewayService/GatewayDescription.cpp \
	../GatewayService/GatewayDescriptionParser.cpp \
	GatewayServiceCfgGenerator.cpp \
	LogicModulesInfoWriter.cpp \
	ConnectionsInfoWriter.cpp \
	SoftwareSettingsGetter.cpp \
    Builder.cpp \
	AppSignalProperties.cpp \
	CodeChecker.cpp \
	CodeOptimization.cpp \
	DbMetrologyConnection.cpp \
    ConnectionStorage.cpp \
    IssueLogger.cpp \
    BuildWorkerThread.cpp \
    BuildResultWriter.cpp \
    Loopbacks.cpp \
    ModuleFirmwareWriter.cpp \
    Parser.cpp \
    ReportAppSignalProvider.cpp \
    ReportGenerator.cpp \
    SignalSet.cpp \
    Busses.cpp \
    OptoModule.cpp \
	RawDataDescription.cpp \
    SignalsHeap.cpp \
    SubsystemStorage.cpp \
    TestSuiteCfgGenerator.cpp \
    UalItems.cpp \
    LmMemoryMap.cpp \
    ConfigurationBuilder.cpp \
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
	Context.cpp

HEADERS += \
    ../lib/ClientBehavior.h \
    ../lib/ConstStrings.h \
	AppLogicCode.h \
	AppLogicCompiler.h \
	CodeItem.h \
	DeviceHelper.h \
	LanControllerInfoHelper.h \
    ../lib/LogicModuleSet.h \
    ../lib/DataSource.h \
    ../lib/Tuning/TuningFilter.h \
	../lib/TuningDataStorage.h \
	../TuningService/TuningSource.h \
	../Metrology/MetrologySignal.h \
	../Metrology/MetrologyConnection.h \
	../Metrology/UnitsConvertor.h \
	../Metrology/UnitsConvertorTable.h \
	../GatewayService/GatewayDescription.h \
	../GatewayService/GatewayDescriptionParser.h \
	GatewayServiceCfgGenerator.h \
	LogicModulesInfoWriter.h \
	SoftwareSettingsGetter.h \
	ConnectionsInfoWriter.h \
	Builder.h \
	AppSignalProperties.h \
	CodeChecker.h \
	CodeOptimization.h \
	DbMetrologyConnection.h \
    ConnectionStorage.h \
    Loopbacks.h \
    ReportAppSignalProvider.h \
    ReportGenerator.h \
    SignalsHeap.h \
    Stable.h \
    IssueLogger.h \
	BuildWorkerThread.h \
    BuildResultWriter.h \
    ModuleFirmwareWriter.h \
    Parser.h \
    SignalSet.h \
    Busses.h \
    OptoModule.h \
    RawDataDescription.h \
    SubsystemStorage.h \
    TestSuiteCfgGenerator.h \
    UalItems.h \
    LmMemoryMap.h \
    TuningBuilder.h \
    ConfigurationBuilder.h \
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
	Context.h

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


