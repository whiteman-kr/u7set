cmake_minimum_required(VERSION 3.16)

# Git based 
# CI_PIPELINE_ID (2570) CI_COMMIT_SHA
#
project(
	u7set
	VERSION 00.00.00.0000
	DESCRIPTION "Radiy Platform Configuration Tools"
	HOMEPAGE_URL "radiy.com")

message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add LTO for Rlease and Windows only. 
# It is diasbale for Unix to speed up CI bulid.
#
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Where to put binaries.
#
if (DEFINED BIN_OUT_DIR)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT_DIR})
	unset(BIN_OUT_DIR)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin/minsizerel)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/relwithdebuginfo)
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

add_subdirectory(ServiceControlManager)
add_subdirectory(ConfigurationService)
add_subdirectory(AppDataService)
add_subdirectory(ArchivingService)
add_subdirectory(TuningService)
add_subdirectory(DiagDataService)

# Do not compile these projects if code coverage is on.
#
if(NOT CODE_COVERAGE)
    add_subdirectory(./Tools/qtkeychain)
    add_subdirectory(u7)

	add_subdirectory(Monitor)
	add_subdirectory(TuningClient)

	add_subdirectory(Metrology)
	add_subdirectory(mconf)	

	add_subdirectory(./Tools/PacketSource)
	add_subdirectory(./Tools/PacketViewer)	
endif()

add_subdirectory(TestSuiteLib)
add_subdirectory(TestSuite)
add_subdirectory(TestSuiteConsole)

add_subdirectory(./Test/googletest)
add_subdirectory(./Test/UalTester)
add_subdirectory(./Test/ClientTests)
add_subdirectory(./Test/SimulatorTests)
add_subdirectory(./Test/u7databaseTests)
add_subdirectory(./Test/MetrologyTests)
#add_subdirectory(./Test/TestSuiteTest)



