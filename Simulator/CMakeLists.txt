cmake_minimum_required(VERSION 3.16)

project(Simulator LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

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

add_library(Simulator STATIC
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

target_precompile_headers(Simulator PRIVATE
	<bit>
	<memory>
	<map>
	<unordered_map>
	<optional>
	<queue>
	<array>
	<vector>
	<queue>
	<algorithm>
	<cstring>
	<filesystem>

	<QtCore>
	<QtQml>
	<QtNetwork>
	<QtConcurrent>
	<QtXml>
	<QVector>
)

target_link_libraries(Simulator PUBLIC Qt6::Xml Qt6::Qml Qt6::Core Qt6::Concurrent Qt6::Network)
target_link_libraries(Simulator PUBLIC CommonLib Protobuf UtilsLib AppSignalLib HardwareLib OnlineLib TrendView)

target_compile_definitions(Simulator PRIVATE SIMULATOR_LIBRARY)
target_include_directories(Simulator INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


