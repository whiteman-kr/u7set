cmake_minimum_required(VERSION 3.16)

# Version, example: 23.6.1.2570
# Year (two last digits) / Month (1-12) / PatchNo (0-...) / PipelineID (2000-...)
#

# Version, example: 385.1.2570
# Project DB Version / PatchNo (0-...) / PipelineID (2000-...)
# 

# Git based 
# CI_PIPELINE_ID (2570) CI_COMMIT_SHA
#
project(
	u7set
	VERSION 00.00.00.0000
	DESCRIPTION "Radiy Platform Configuration Tools"
	HOMEPAGE_URL "radiy.com")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Where to put binaries.
#
if (DEFINED BIN_OUT_DIR)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT_DIR})
	unset(BIN_OUT_DIR)
else()
	# ${CMAKE_BUILD_TYPE} can be empty, for example in VS Code, which makes configuration for multiple types?
	#
	string(TOLOWER "${CMAKE_BUILD_TYPE}" LOWERCASE_BUILD_TYPE)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${LOWERCASE_BUILD_TYPE})
endif()

option(CODE_COVERAGE "Code coverage (Linux only)" OFF)
include(./cmake/codecoverage.cmake)

message("-----------------------------------------")
message(${PROJECT_NAME})
message("Version: " ${CMAKE_PROJECT_VERSION})
message("Build type: " ${CMAKE_BUILD_TYPE})
message("Build output dir set to: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message("Code coverage: " ${CODE_COVERAGE})
message("-----------------------------------------")

# Add projects.
#
add_subdirectory(Protobuf)
add_subdirectory(CommonLib)
add_subdirectory(UtilsLib)
add_subdirectory(HardwareLib)
add_subdirectory(AppSignalLib)
add_subdirectory(OnlineLib)
add_subdirectory(DbLib)
add_subdirectory(ServiceLib)
add_subdirectory(ClientLib)

add_subdirectory(TrendView)
add_subdirectory(VFrame30)

add_subdirectory(Simulator)
add_subdirectory(SimulatorConsole)

add_subdirectory(Builder)
add_subdirectory(BuilderConsole)

add_subdirectory(./Tools/qtkeychain)
add_subdirectory(u7)

add_subdirectory(ServiceControlManager)
add_subdirectory(ConfigurationService)
add_subdirectory(AppDataService)
add_subdirectory(ArchivingService)
add_subdirectory(TuningService)
add_subdirectory(DiagDataService)

add_subdirectory(Monitor)
add_subdirectory(TuningClient)

add_subdirectory(Metrology)
add_subdirectory(mconf)

add_subdirectory(TestSuiteLib)
add_subdirectory(TestSuite)
add_subdirectory(TestSuiteConsole)

add_subdirectory(./Tools/PacketSource)
add_subdirectory(./Tools/PacketViewer)

add_subdirectory(./Test/googletest)
add_subdirectory(./Test/UalTester)
add_subdirectory(./Test/ClientTests)
add_subdirectory(./Test/SimulatorTests)
add_subdirectory(./Test/u7databaseTests)
add_subdirectory(./Test/MetrologyTests)
#add_subdirectory(./Test/TestSuiteTest)



