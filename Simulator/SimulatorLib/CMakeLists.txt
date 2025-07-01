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
	./src/SimAfb.cpp
	./src/SimAfb.h
	./src/SimAppDataLanInterface.cpp
	./src/SimAppDataLanInterface.h
	./src/SimAppDataTransmitter.cpp
	./src/SimAppDataTransmitter.h
	./src/SimAppSignalManager.cpp
	./src/SimAppSignalManagerImpl.cpp
	./src/SimAppSignalManagerImpl.h
	./src/SimCommandProcessor.cpp
	./src/SimCommandProcessor.h
	./src/SimCommandProcessor_LM5_LM6.cpp
	./src/SimCommandProcessor_LM5_LM6.h
	./src/SimConnections.cpp
	./src/SimConnectionsImpl.cpp
	./src/SimConnectionsImpl.h
	./src/SimConsoleLogFile.cpp
	./src/SimControl.cpp
	./src/SimControlImpl.cpp
	./src/SimControlImpl.h
	./src/SimControlStatus.cpp
	./src/SimDeviceEmulator.cpp
	./src/SimDeviceEmulator.h
	./src/SimDeviceState.cpp
	./src/SimDiagDataLanInterface.cpp
	./src/SimDiagDataLanInterface.h
	./src/SimEeprom.cpp
	./src/SimEeprom.h
	./src/SimException.cpp
	./src/SimException.h
	./src/SimLanInterface.cpp
	./src/SimLanInterface.h
	./src/SimLans.cpp
	./src/SimLans.h
	./src/SimLogicModule.cpp
	./src/SimLogicModuleImpl.cpp
	./src/SimLogicModuleImpl.h
	./src/SimOverrideSignals.cpp
	./src/SimOverrideSignalsImpl.cpp
	./src/SimOverrideSignalsImpl.h
	./src/SimProfiles.cpp
	./src/SimRam.cpp
	./src/SimScopedLog.cpp
	./src/SimScopedLog.h
	./src/SimScriptConnection.cpp
	./src/SimScriptConnection.h
	./src/SimScriptDevUtils.cpp
	./src/SimScriptDevUtils.h
	./src/SimScriptLmDescription.cpp
	./src/SimScriptLmDescription.h
	./src/SimScriptLogicModule.cpp
	./src/SimScriptLogicModule.h
	./src/SimScriptRamAddress.cpp
	./src/SimScriptRamAddress.h
	./src/SimScriptSignal.cpp
	./src/SimScriptSignal.h
	./src/SimScriptSimulator.cpp
	./src/SimScriptSimulator.h
	./src/SimService.cpp
	./src/SimSnapshot.cpp
	./src/SimSnapshot.h
	./src/SimSoftware.cpp
	./src/SimSoftwareImpl.cpp
	./src/SimSoftwareImpl.h
	./src/SimSubsystem.cpp
	./src/SimSubsystemImpl.cpp
	./src/SimSubsystemImpl.h
	./src/SimTestObserver.cpp
	./src/SimTestObserver.h
	./src/SimTuningLanInterface.cpp
	./src/SimTuningLanInterface.h
	./src/SimTuningRecord.cpp
	./src/SimTuningRecord.h
	./src/SimTuningServiceCommunicator.cpp
	./src/SimTuningServiceCommunicator.h
	./src/SimTuningSignalManager.cpp
	./src/SimTuningSignalManager.h
	./src/Simulator.cpp
	./src/SimulatorPrivate.cpp
	./src/SimulatorPrivate.h
	./src/SoftwareXmlReader.cpp
	./src/SoftwareXmlReader.h

	./include/SimulatorLib/SimAppSignalManager.h
	./include/SimulatorLib/SimConnections.h
	./include/SimulatorLib/SimConsoleLogFile.h
	./include/SimulatorLib/SimControl.h
	./include/SimulatorLib/SimControlStatus.h
	./include/SimulatorLib/SimDeviceCommand.h
	./include/SimulatorLib/SimDeviceState.h
	./include/SimulatorLib/SimLogicModule.h
	./include/SimulatorLib/SimOverrideRamRecord.h
	./include/SimulatorLib/SimOverrideSignals.h
	./include/SimulatorLib/SimProfiles.h
	./include/SimulatorLib/SimRam.h
	./include/SimulatorLib/SimScriptItem.h
	./include/SimulatorLib/SimService.h
	./include/SimulatorLib/SimSoftware.h
	./include/SimulatorLib/SimSubsystem.h
	./include/SimulatorLib/Simulator.h

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
target_link_libraries(${TARGET} PUBLIC CommonLib Behavior ProtobufEx UtilsLib AppSignalLib HardwareLib OnlineLib ClientLib TrendView SimulatorGrpc)

target_compile_definitions(${TARGET} PRIVATE SIMULATOR_LIBRARY)

target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
