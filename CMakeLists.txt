cmake_minimum_required(VERSION 3.16)

# Creates variables:
#	U7SET_MAJOR_VERSION, 
#	U7SET_MINOR_VERSION, 
#	U7SET_PATCH_VERSION, 
#	U7SET_BRANCH_NAME,
#	U7SET_RELEASE_TYPE
#
include(cmake/version.cmake)

# Git based 
#
project(
	u7set
	VERSION ${U7SET_MAJOR_VERSION}.${U7SET_MINOR_VERSION}.${U7SET_PATCH_VERSION}
	DESCRIPTION "Radiy Platform Configuration Tools"
	HOMEPAGE_URL "radiy.com")

message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the Qt version and find required components.
# Finding components here speeds up cmake configure step.
#
set(QT_VERSION "6.5.3")

find_package(Qt6 ${QT_VERSION} REQUIRED COMPONENTS Charts Concurrent Core DBus Gui LinguistTools Network PrintSupport Qml SerialPort Sql Svg Test Widgets Xml)

if (MSVC)
	find_package(Qt6 REQUIRED COMPONENTS AxContainer) # Metrology
endif (MSVC)

# Set warnings level and exclusions.
#
include(cmake/warnings.cmake)
set_warnings()

# Add LTO for Release and Windows only. 
# It is diasbale for Unix to speed up CI bulid.
#
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Where to put binaries.
#
if (DEFINED BIN_OUT_DIR)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_OUT_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_OUT_DIR})
else()
	string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWERCASE)

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE_LOWERCASE})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${BUILD_TYPE_LOWERCASE})
endif()

option(CODE_COVERAGE "Code coverage (Linux only)" OFF)
include(./cmake/codecoverage.cmake)

# AddressSanitizer
#
option(ADDRESS_SANITIZER "Address Sanitizer" OFF)

if(ADDRESS_SANITIZER)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif()

# UndfineBehaviorSanitizer
#
option(UB_SANITIZER "UB Sanitizer" OFF)

if(UB_SANITIZER)
	add_compile_options(-fsanitize=undefined)
	add_link_options(-fsanitize=undefined)
endif()

# Update Translations
# Updates .ts files
#
option(UPDATE_TRANSLATIONS "Update Translations" OFF)

message("-----------------------------------------")
message(${PROJECT_NAME})
message("Version: ${CMAKE_PROJECT_VERSION} (${U7SET_BRANCH_NAME})")
message("Release type: ${U7SET_RELEASE_TYPE}")
if(CMAKE_CONFIGURATION_TYPES)
	message("Build types: ${CMAKE_CONFIGURATION_TYPES}")
	message(FATAL_ERROR "Multiple configuration is not supported.")
else()
	message("Build type: " ${CMAKE_BUILD_TYPE})
endif()
message("Build output dir set to: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message("Code coverage: " ${CODE_COVERAGE})
message("AddressSanitizer: " ${ADDRESS_SANITIZER})
message("UBSanitizer: " ${UB_SANITIZER})
message("Update translations: " ${UPDATE_TRANSLATIONS})
message("CMake executable:  ${CMAKE_COMMAND}")
message("CMake version:  ${CMAKE_VERSION}")
message("Current file path:  ${CMAKE_CURRENT_LIST_FILE}")
message("User: ${U7SET_USER_NAME}")
message("Host: ${U7SET_HOSTNAME}")
message("Qt Version: ${Qt6_VERSION}")
message("Qt Path: ${Qt6_DIR}")
message("-----------------------------------------")

# Add projects.
#
add_subdirectory(Protobuf)
add_subdirectory(CommonLib)
add_subdirectory(UtilsLib)
add_subdirectory(AppSignalLib)
add_subdirectory(OnlineLib)
add_subdirectory(ServiceLib)
add_subdirectory(ClientLib)
add_subdirectory(ReportLib)

add_subdirectory(libs)

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
add_subdirectory(GatewayService)
add_subdirectory(DiagDataService)

# Do not compile these projects if code coverage is on.
#
if(NOT CODE_COVERAGE)
    add_subdirectory(./Tools/qtkeychain)
    add_subdirectory(u7)

	add_subdirectory(Monitor)
	add_subdirectory(TuningClient)
	add_subdirectory(Diagnostics)

	add_subdirectory(Metrology)
	add_subdirectory(mconf)	

	add_subdirectory(./Tools/PacketSource)
	add_subdirectory(./Tools/PacketViewer)	
endif()

add_subdirectory(TestSuiteLib)
add_subdirectory(TestSuite)
add_subdirectory(TestSuiteConsole)

add_subdirectory(./Test/googletest)
#add_subdirectory(./Test/UalTester)
add_subdirectory(./Test/ClientTests)
add_subdirectory(./Test/SimulatorTests)
add_subdirectory(./Test/u7databaseTests)
add_subdirectory(./Test/MetrologyTests)
#add_subdirectory(./Test/TestSuiteTest)

# Create install target.
# Just run /u7setinstall/createinstall.bat file.
#
add_custom_target(create_install
	COMMAND ${CMAKE_COMMAND} -E env "U7SET_FULL_VERSION=${U7SET_FULL_VERSION}" "${CMAKE_CURRENT_SOURCE_DIR}/u7setinstall/createinstall.bat"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/u7setinstall"
	COMMENT "Running createinstall.bat with U7SET_FULL_VERSION=${U7SET_FULL_VERSION}"
)
