cmake_minimum_required(VERSION 3.16)

project(Builder LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml Xml Sql Concurrent Gui PrintSupport)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

add_library(Builder STATIC
	../lib/ClientBehavior.cpp
	../lib/ClientBehavior.h
	../lib/ConstStrings.h
	../lib/DataSource.cpp
	../lib/DataSource.h
	../lib/LogicModuleSet.cpp
	../lib/LogicModuleSet.h
	../lib/Tuning/TuningFilter.cpp
	../lib/Tuning/TuningFilter.h
	../lib/TuningDataStorage.cpp
	../lib/TuningDataStorage.h
	../Metrology/MetrologyConnection.cpp
	../Metrology/MetrologyConnection.h
	../Metrology/MetrologySignal.cpp
	../Metrology/MetrologySignal.h
	../Metrology/UnitsConvertor.cpp
	../Metrology/UnitsConvertor.h
	../Metrology/UnitsConvertorTable.cpp
	../Metrology/UnitsConvertorTable.h
	../TuningService/TuningSource.cpp
	../TuningService/TuningSource.h
	AppDataServiceCfgGenerator.cpp
	AppDataServiceCfgGenerator.h
	ApplicationLogicCode.cpp
	ApplicationLogicCode.h
	ApplicationLogicCompiler.cpp
	ApplicationLogicCompiler.h
	AppSignalProperties.cpp
	AppSignalProperties.h
	ArchivingServiceCfgGenerator.cpp
	ArchivingServiceCfgGenerator.h
	BdfFile.cpp
	BdfFile.h
	Builder.cpp
	Builder.h
	BuildResultWriter.cpp
	BuildResultWriter.h
	BuildWorkerThread.cpp
	BuildWorkerThread.h
	Busses.cpp
	Busses.h
	CodeChecker.cpp
	CodeChecker.h
	CodeOptimization.cpp
	CodeOptimization.h
	ConfigurationBuilder.cpp
	ConfigurationBuilder.h
	ConfigurationServiceCfgGenerator.cpp
	ConfigurationServiceCfgGenerator.h
	ConnectionsInfoWriter.cpp
	ConnectionsInfoWriter.h
	ConnectionStorage.cpp
	ConnectionStorage.h
	Context.cpp
	Context.h
	DbMetrologyConnection.cpp
	DbMetrologyConnection.h
	DeviceHelper.cpp
	DeviceHelper.h
	DiagDataServiceCfgGenerator.cpp
	DiagDataServiceCfgGenerator.h
	FbParamCalculation.cpp
	IssueLogger.cpp
	IssueLogger.h
	LanControllerInfoHelper.cpp
	LanControllerInfoHelper.h
	LmDescriptionSet.cpp
	LmDescriptionSet.h
	LmMemoryMap.cpp
	LmMemoryMap.h
	LogicModulesInfoWriter.cpp
	LogicModulesInfoWriter.h
	Loopbacks.cpp
	Loopbacks.h
	MemWriteMap.cpp
	MemWriteMap.h
	MetrologyCfgGenerator.cpp
	MetrologyCfgGenerator.h
	ModuleFirmwareWriter.cpp
	ModuleFirmwareWriter.h
	ModuleLogicCompiler.cpp
	ModuleLogicCompiler.h
	ModulesRawData.cpp
	ModulesRawData.h
	MonitorCfgGenerator.cpp
	MonitorCfgGenerator.h
	OptoModule.cpp
	OptoModule.h
	Parser.cpp
	Parser.h
	RawDataDescription.cpp
	RawDataDescription.h
	ReportAppSignalProvider.cpp
	ReportAppSignalProvider.h
	ReportGenerator.cpp
	ReportGenerator.h
	RunOrder.cpp
	RunOrder.h
	SignalSet.cpp
	SignalSet.h
	SignalsHeap.cpp
	SignalsHeap.h
	SoftwareCfgGenerator.cpp
	SoftwareCfgGenerator.h
	SoftwareSettingsGetter.cpp
	SoftwareSettingsGetter.h
	SubsystemStorage.cpp
	SubsystemStorage.h
	TestClientCfgGenerator.cpp
	TestClientCfgGenerator.h
	TestSuiteCfgGenerator.cpp
	TestSuiteCfgGenerator.h
	TuningBuilder.cpp
	TuningBuilder.h
	TuningClientCfgGenerator.cpp
	TuningClientCfgGenerator.h
	TuningServiceCfgGenerator.cpp
	TuningServiceCfgGenerator.h
	UalItems.cpp
	UalItems.h
)

target_precompile_headers(Builder PRIVATE
	<cassert>
	<algorithm>
	<functional>
	<list>
	<map>
	<memory>
	<optional>
	<ranges>
	<set>
	<typeindex>
	<unordered_map>
	<unordered_set>
	<vector>

	<QtCore>
	<QtGui>
	<QtQml>
	<QtConcurrent>
	<QVector>
	<QWidget>
	<QFile>
)

target_link_libraries(Builder PRIVATE Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Sql Qt6::Concurrent Qt6::Gui Qt6::PrintSupport)
target_link_libraries(Builder PUBLIC Protobuf HardwareLib DbLib AppSignalLib OnlineLib UtilsLib CommonLib VFrame30 Simulator)

target_compile_definitions(Builder PRIVATE BUILDER_LIBRARY)
target_compile_definitions(Builder PRIVATE IS_BUILDER)
#target_include_directories(Builder INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


