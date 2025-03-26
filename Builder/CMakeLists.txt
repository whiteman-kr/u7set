cmake_minimum_required(VERSION 3.16)

project(Builder LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml Xml Sql Concurrent Gui PrintSupport)
find_package(Boost REQUIRED COMPONENTS graph)
find_package(Lua REQUIRED)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

set(TARGET Builder)

qt_add_library(${TARGET} STATIC
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
	./Vdu/VduAppSignals.h
	./Vdu/VduAppSignalsInfoGenerator.cpp
	./Vdu/VduAppSignalsInfoGenerator.h
	./Vdu/VduConfigFile.cpp
	./Vdu/VduConfigFile.h
	./Vdu/VduFontGenerator.cpp
	./Vdu/VduFontGenerator.h
	./Vdu/VduFontProvider.cpp
	./Vdu/VduFontProvider.h
	./Vdu/VduLuaScript.cpp
	./Vdu/VduLuaScript.h
	./Vdu/VduSchemaFile.h
	./Vdu/VduSchemaGenerator.cpp
	./Vdu/VduSchemaGenerator.h
	./Vdu/VduTypes.h
	./Vdu/VduUnicodeSubsets.h
	AdsBridgeCfgGenerator.cpp
	AdsBridgeCfgGenerator.h
	AppDataServiceCfgGenerator.cpp
	AppDataServiceCfgGenerator.h
	AppLogicCode.cpp
	AppLogicCode.h
	AppLogicCompiler.cpp
	AppLogicCompiler.h
	AppSignalListStorage.cpp
	AppSignalListStorage.h
	AppSignalProperties.cpp
	AppSignalProperties.h
	ArchivingServiceCfgGenerator.cpp
	ArchivingServiceCfgGenerator.h
	BdfFile.cpp
	BdfFile.h
	Builder.cpp
	Builder.h
	BuildOptions.cpp
	BuildOptions.h
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
	GatewayDescriptionParser.cpp
	GatewayDescriptionParser.h
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
	LogicModuleSet.cpp
	LogicModuleSet.h
	LogicModulesInfoWriter.cpp
	LogicModulesInfoWriter.h
	Loopbacks.cpp
	Loopbacks.h
	MetrologyCfgGenerator.cpp
	MetrologyCfgGenerator.h
	ModuleFirmwareWriter.cpp
	ModuleFirmwareWriter.h
	ModuleLogicCompiler.cpp
	ModuleLogicCompiler.h
	ModulesRawData.cpp
	ModulesRawData.h
	ModulesReportGenerator.cpp
	ModulesReportGenerator.h
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
	TaskGetEquipment.cpp
	TaskGetEquipment.h
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
	UuidGenerator.h
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

	<QCryptographicHash>
	<QDebug>
	<QDir>
	<QDomDocument>
	<QFile>
	<QFileInfo>
	<QHostAddress>
	<QHostInfo>
	<QImage>
	<QJSEngine>
	<QJsonArray>
	<QJsonDocument>
	<QJsonObject>
	<QJSValueIterator>
	<QPageSize>
	<QPainter>
	<QQmlEngine>
	<QStandardPaths>
	<QtConcurrent>
	<QtEndian>
	<QVector>
	<QWidget>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	<CommonLib/AfbParamValue.h>
	
	<ProtoSerialization.h>
	<ProtoCommonHelper.h>
	<AppSignal.pb.h>
	<Bus.pb.h>
	<Hardware.pb.h>
	<Network.pb.h>
	
	"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
	"../AppSignalLib/TuningValue.h"

	<DbLib/DbController.h>
	<DbLib/DbObjectStorage.h>
)

target_link_libraries(${TARGET} PUBLIC Boost::graph)
target_link_libraries(${TARGET} PUBLIC Qt6::Widgets Qt6::Qml Qt6::Xml Qt6::Sql Qt6::Concurrent Qt6::Gui Qt6::PrintSupport)
target_link_libraries(${TARGET} PUBLIC CommonLib Behavior ProtobufEx UtilsLib HardwareLib DbLib AppSignalLib OnlineLib VFrame30 SimulatorLib ReportLib AppSignalLists TuningLib GatewayLib)

# Lua library
#
target_include_directories(${TARGET} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${TARGET} PRIVATE ${LUA_LIBRARIES})

# Add the IS_BUILDER definition
#
target_compile_definitions(${TARGET} PRIVATE IS_BUILDER)

# Add the CODE_COVERAGE option
#
add_code_coverage_options(${TARGET} CODE_COVERAGE)
