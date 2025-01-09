cmake_minimum_required(VERSION 3.16)

# Creates variables:
#	U7SET_MAJOR_VERSION, 
#	U7SET_MINOR_VERSION, 
#	U7SET_PATCH_VERSION, 
#	U7SET_BRANCH_NAME,
#	U7SET_RELEASE_TYPE
#
include(cmake/version.cmake)

# Emit error if VCPKG_ROOT is not set
#
if(NOT DEFINED ENV{VCPKG_ROOT})
	message(FATAL_ERROR "VCPKG_ROOT is not set. Please set it to the vcpkg root directory.")
endif()

if (UNIX)
	set(VCPKG_TARGET_TRIPLET "x64-linux")
endif()

set(VCPKG_ROOT $ENV{VCPKG_ROOT})
include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

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
find_package(Boost REQUIRED COMPONENTS graph)
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

if (WIN32)
	find_package(Qt6 REQUIRED COMPONENTS AxContainer) # Metrology
endif (WIN32)

# Set warnings level and exclusions.
#
include(cmake/warnings.cmake)
set_warnings()

# Add LTO for Release and Windows only. 
# It is diasabled for Unix to speed up CI bulid.
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

# Allow usage FTDI functions - is used for getting module configurator serial number.
#
option(ALLOW_FTDI "Allow FTDI" OFF)

# Build Licenser
#
option(BUILD_LICENSER "Build Licenser" OFF)

if (DEFINED ENV{CI_BUILD_LICENSER})
	set(BUILD_LICENSER ON)
endif()

# ---
#
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
message("VCPKG_ROOT: ${VCPKG_ROOT}")
message("Allow FTDI: ${ALLOW_FTDI}")
message("Build Licenser: ${BUILD_LICENSER}")
message("-----------------------------------------")

# Add projects.
#
add_subdirectory(UtilsLib)
add_subdirectory(AppSignalLib)
add_subdirectory(OnlineLib)

add_subdirectory(libs)

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

add_subdirectory(TestSuiteConsole)

# Do not compile these projects if code coverage is on.
#
if(NOT CODE_COVERAGE)
    add_subdirectory(u7)
	add_subdirectory(Simulator)
	add_subdirectory(TestSuite)
	add_subdirectory(mconf)	
	add_subdirectory(./Tools/qtkeychain)

	add_subdirectory(Monitor)
	add_subdirectory(TuningClient)
	add_subdirectory(Diagnostics)
	add_subdirectory(Metrology)
endif()

add_subdirectory(Tests)

if (BUILD_LICENSER)
	add_subdirectory(./Tools/Licenser)
endif()

# Create install target.
# Just run /u7setinstall/createinstall.bat file.
#
add_custom_target(create_install
	COMMAND ${CMAKE_COMMAND} -E env "U7SET_FULL_VERSION=${U7SET_FULL_VERSION}" "${CMAKE_CURRENT_SOURCE_DIR}/u7setinstall/createinstall.bat"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/u7setinstall"
	COMMENT "Running createinstall.bat with U7SET_FULL_VERSION=${U7SET_FULL_VERSION}"
)

# Generate dependency graphs into the build directory/LibraryDependencies
#
if(WIN32)
	# Add a custom target to run the dependencies.bat script
	add_custom_target(GenerateDependencySvgs
		COMMAND "${CMAKE_SOURCE_DIR}/dependencies.bat"
		COMMENT "Generating library dependency SVG files..."
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
endif()