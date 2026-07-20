cmake_minimum_required(VERSION 3.16)

project(PluginTO5 LANGUAGES CXX)

include(../../cmake/vld.cmake)
include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (MSVC)
  # MSVC warning suppressions
  add_compile_options(
	/wd4828 # The file contains a character starting at offset 0x10d2 that is illegal in the current source character set (codepage 65001).
  )
endif (MSVC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Xml PrintSupport Test)
qt_standard_project_setup()

set (TC_SOURCES
	main.cpp
	PluginTO5MainWindow.cpp
	PluginTO5MainWindow.h
	ComparatorsStorage.cpp
	ComparatorsStorage.h
	DialogImport.cpp
	DialogImport.h
	TO5Runner.cpp
	TO5Runner.h
	TO5Settings.cpp
	TO5Settings.h
)

set (TC_RESOURCES
	Resources.qrc
#	../../Etc/Logo/RadiyLogo.qrc
)

set (TC_FORMS
#	DialogSettings.ui
)

# Add icon
#
if (MSVC)
#	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set(TARGET PluginTO5)

add_executable(${TARGET} WIN32
	${TC_FORMS}
	${TC_SOURCES}
	${TC_RESOURCES}
	${WINDOWS_RESOURCES}
)

# Reuse precompiled headers from SchemaClientLib
#
target_precompile_headers(${TARGET} REUSE_FROM SchemaClientLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

# Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Xml Qt6::PrintSupport Qt6::Test)
target_link_libraries(${TARGET} PRIVATE CommonLib UtilsLib AppSignalLib OnlineLib ClientLib AdsConnectionLib TestSuiteLib ReportLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
