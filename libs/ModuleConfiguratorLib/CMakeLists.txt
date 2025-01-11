cmake_minimum_required(VERSION 3.28)

project(ModuleConfiguratorLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET ModuleConfiguratorLib)

if(ALLOW_FTDI AND MSVC)
	set(FTDI_FILES ftdi/ftd2xx.h)
endif()

qt_add_library(${TARGET} STATIC
	${FTDI_FILES}
	./include/ModuleConfiguratorLib/Configurator.h
	Configurator.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<cstring>
	<optional>
	<vector>
	<QApplication>
	<QDir>
	<QHostInfo>
	<QMutex>
	<QSerialPort>
	<QString>
	<QtEndian>
	<QUuid>

	<CommonLib/Types.h>
	<HardwareLib/ModuleFirmware.h>
)

target_compile_definitions(${TARGET} PRIVATE MODULECONFIGURATION_LIB_DOMAIN)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::SerialPort)
target_link_libraries(${TARGET} PUBLIC CommonLib HardwareLib UtilsLib LicenseLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(ALLOW_FTDI AND MSVC)
	# add ALLOW_FTDI preprocessor definition
	#
	target_compile_definitions(${TARGET} PRIVATE ALLOW_FTDI)

	# Link FTDI library
	#
	add_library(ftdi STATIC IMPORTED)
	set_target_properties(ftdi PROPERTIES
	  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ftdi64/ftd2xx.lib"
	  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/ftdi"
	)

	target_link_libraries(${TARGET} PRIVATE ftdi)
endif()

# DLL for FTDI
#
if (MSVC)
	set(DLL_COPY_FILE_LIST
		${CMAKE_CURRENT_SOURCE_DIR}/../../bin_Win64/ftd2xx.dll
	)

	add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL_COPY_FILE_LIST} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
