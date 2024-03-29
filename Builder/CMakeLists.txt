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

set(TARGET Builder)

qt_add_library(${TARGET} STATIC
	../GatewayService/GatewayDescription.cpp
	../GatewayService/GatewayDescription.h
	../GatewayService/GatewayDescriptionParser.cpp
	../GatewayService/GatewayDescriptionParser.h
	../GatewayService/IvsImpulseGateway.cpp
	../GatewayService/IvsImpulseGateway.h
	../GatewayService/ModbusTcpSlaveGateway.cpp
	../GatewayService/ModbusTcpSlaveGateway.h
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
	../Metrology/UnitsConverter.cpp
	../Metrology/UnitsConverter.h
	../Metrology/UnitsConverterTable.cpp
	../Metrology/UnitsConverterTable.h
	../TuningService/TuningSource.cpp
	../TuningService/TuningSource.h
	./Vdu/VduSchemaFile.h
	./Vdu/VduSchemaGenerator.cpp
	./Vdu/VduSchemaGenerator.h
	AppDataServiceCfgGenerator.cpp
	AppDataServiceCfgGenerator.h
	AppLogicCode.cpp
	AppLogicCode.h
	AppLogicCompiler.cpp
	AppLogicCompiler.h
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
	CodeItem.cpp
	CodeItem.h
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
	DbMatsUsers.cpp
	DbMatsUsers.h
	DbMetrologyConnection.cpp
	DbMetrologyConnection.h
	DeviceHelper.cpp
	DeviceHelper.h
	DiagDataServiceCfgGenerator.cpp
	DiagDataServiceCfgGenerator.h
	DiagnosticsCfgGenerator.cpp
	DiagnosticsCfgGenerator.h
	DiagSignalTypesStorage.cpp
	DiagSignalTypesStorage.h
	FbParamCalculation.cpp
	GatewayServiceCfgGenerator.cpp
	GatewayServiceCfgGenerator.h
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
	RunOrder.cpp
	RunOrder.h
	SchemasReportGenerator.cpp
	SchemasReportGenerator.h
	ScriptChecker.cpp
	ScriptChecker.h
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

target_precompile_headers(${TARGET} PRIVATE
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

	<QDomDocument>
	<QFile>
	<QHostInfo>
	<QtConcurrent>
	<QVector>
	<QWidget>
	<QJSEngine>

	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"
	"../CommonLib/AfbParamValue.h"

	"../Proto/ProtoSerialization.h"
	"../Proto/ProtoCommonHelper.h"
	"../Proto/Network.pb.h"
	"../Proto/AppSignal.pb.h"
	"../Proto/Bus.pb.h"

	"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
	"../AppSignalLib/TuningValue.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Sql Qt6::Concurrent Qt6::Gui Qt6::PrintSupport)
target_link_libraries(${TARGET} PUBLIC CommonLib Behavior Protobuf UtilsLib HardwareLib DbLib AppSignalLib OnlineLib VFrame30 Simulator ReportLib)

target_compile_definitions(${TARGET} PRIVATE IS_BUILDER)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
