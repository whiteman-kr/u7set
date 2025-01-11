cmake_minimum_required(VERSION 3.16)

project(SimulatorLib LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Xml Qml Core Concurrent Network)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

set(TARGET SimulatorLib)

qt_add_library(${TARGET} STATIC
	SimAfb.cpp
	SimAfb.h
	SimAppDataLanInterface.cpp
	SimAppDataLanInterface.h
	SimAppDataTransmitter.cpp
	SimAppDataTransmitter.h
	SimAppSignalManagerImpl.cpp
	SimAppSignalManagerImpl.h
	SimCommandProcessor.cpp
	SimCommandProcessor.h
	SimCommandProcessor_LM5_LM6.cpp
	SimCommandProcessor_LM5_LM6.h
	SimConnections.cpp
	SimConsoleLogFile.cpp
	SimControlImpl.cpp
	SimControlImpl.h
	SimControl.cpp
	SimDeviceEmulator.cpp
	SimDeviceEmulator.h
	SimDiagDataLanInterface.cpp
	SimDiagDataLanInterface.h
	SimEeprom.cpp
	SimEeprom.h
	SimException.cpp
	SimException.h
	SimLanInterface.cpp
	SimLanInterface.h
	SimLans.cpp
	SimLans.h
	SimLogicModuleImpl.cpp
	SimLogicModuleImpl.h
	SimOverrideSignalsImpl.cpp
	SimOverrideSignalsImpl.h
	SimProfiles.cpp
	SimRam.cpp
	SimScopedLog.cpp
	SimScopedLog.h
	SimScriptConnection.cpp
	SimScriptConnection.h
	SimScriptDevUtils.cpp
	SimScriptDevUtils.h
	SimScriptLmDescription.cpp
	SimScriptLmDescription.h
	SimScriptLogicModule.cpp
	SimScriptLogicModule.h
	SimScriptRamAddress.cpp
	SimScriptRamAddress.h
	SimScriptSignal.cpp
	SimScriptSignal.h
	SimScriptSimulator.cpp
	SimScriptSimulator.h
	SimSoftwareImpl.cpp
	SimSoftwareImpl.h
	SimSubsystemImpl.cpp
	SimSubsystemImpl.h
	SimTestObserver.cpp
	SimTestObserver.h
	SimTuningLanInterface.cpp
	SimTuningLanInterface.h
	SimTuningRecord.cpp
	SimTuningRecord.h
	SimTuningServiceCommunicator.cpp
	SimTuningServiceCommunicator.h
	SimTuningSignalManager.cpp
	SimTuningSignalManager.h
	Simulator.cpp
	SimulatorPrivate.cpp
	SimulatorPrivate.h
	SoftwareXmlReader.cpp
	SoftwareXmlReader.h
	
	SimConnectionsImpl.h
	SimConnectionsImpl.cpp
	SimControlStatus.cpp
	SimDeviceState.cpp
	SimSoftware.cpp
	SimLogicModule.cpp
	SimSubsystem.cpp
	SimOverrideSignals.cpp
	SimAppSignalManager.cpp

	./include/SimulatorLib/SimScriptItem.h
	./include/SimulatorLib/SimConnections.h
	./include/SimulatorLib/SimControl.h
	./include/SimulatorLib/SimControlStatus.h
	./include/SimulatorLib/SimDeviceState.h
	./include/SimulatorLib/Simulator.h
	./include/SimulatorLib/SimSoftware.h
	./include/SimulatorLib/SimLogicModule.h
	./include/SimulatorLib/SimDeviceCommand.h
	./include/SimulatorLib/SimSubsystem.h
	./include/SimulatorLib/SimOverrideSignals.h
	./include/SimulatorLib/SimOverrideRamRecord.h
	./include/SimulatorLib/SimAppSignalManager.h
	./include/SimulatorLib/SimRam.h
	./include/SimulatorLib/SimProfiles.h
	./include/SimulatorLib/SimConsoleLogFile.h

	SimulatorLib.qrc
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<array>
	<bit>
	<cassert>
	<cfenv>
	<chrono>
	<cmath>
	<concepts>
	<cstddef>
	<cstring>
	<deque>
	<filesystem>
	<functional>
	<limits>
	<map>
	<memory>
	<optional>
	<queue>
	<ranges>
	<type_traits>
	<unordered_map>
	<variant>
	<vector>

	<QException>
	<QJSEngine>
	<QJSValue>
	<QJSValueIterator>
	<QMutex>
	<QtConcurrent>
	<QThread>
	<QUdpSocket>
	<QVector>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	<CommonLib/AfbParamValue.h>
	
	<ProtoSerialization.h>
	<Network.pb.h>
	<AppSignal.pb.h>
	
	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Xml Qt6::Qml Qt6::Core Qt6::Concurrent Qt6::Network)
target_link_libraries(${TARGET} PUBLIC CommonLib Behavior ProtobufEx UtilsLib AppSignalLib HardwareLib OnlineLib ClientLib TrendView)

target_compile_definitions(${TARGET} PRIVATE SIMULATOR_LIBRARY)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
