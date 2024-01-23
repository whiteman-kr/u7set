cmake_minimum_required(VERSION 3.16)

project(Simulator LANGUAGES CXX)

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

set(TARGET Simulator)

qt_add_library(${TARGET} STATIC
	SimAfb.cpp
	SimAfb.h
	SimAppDataLanInterface.cpp
	SimAppDataLanInterface.h
	SimAppDataTransmitter.cpp
	SimAppDataTransmitter.h
	SimAppSignalManager.cpp
	SimAppSignalManager.h
	SimCommandProcessor.cpp
	SimCommandProcessor.h
	SimCommandProcessor_LM5_LM6.cpp
	SimCommandProcessor_LM5_LM6.h
	SimConnections.cpp
	SimConnections.h
	SimConsoleLogFile.cpp
	SimConsoleLogFile.h
	SimControl.cpp
	SimControl.h
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
	SimLogicModule.cpp
	SimLogicModule.h
	SimOverrideSignals.cpp
	SimOverrideSignals.h
	SimProfiles.cpp
	SimProfiles.h
	SimRam.cpp
	SimRam.h
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
	SimSoftware.cpp
	SimSoftware.h
	SimSubsystem.cpp
	SimSubsystem.h
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
	Simulator.h
	SoftwareXmlReader.cpp
	SoftwareXmlReader.h
)

target_precompile_headers(${TARGET} PUBLIC
	<algorithm>
	<array>
	<bit>
	<cfenv>
	<chrono>
	<cmath>
	<cstring>
	<deque>
	<filesystem>
	<map>
	<memory>
	<optional>
	<queue>
	<ranges>
	<unordered_map>
	<vector>

	<QtQml>
	<QtNetwork>
	<QtConcurrent>
	<QtXml>
	<QVector>

	"../AppSignalLib/AppSignalParam.h"
)

target_link_libraries(${TARGET} PUBLIC Qt6::Xml Qt6::Qml Qt6::Core Qt6::Concurrent Qt6::Network)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib AppSignalLib HardwareLib OnlineLib ClientLib TrendView)

target_compile_definitions(${TARGET} PRIVATE SIMULATOR_LIBRARY)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
