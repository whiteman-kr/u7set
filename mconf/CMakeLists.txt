cmake_minimum_required(VERSION 3.16)

project(mconf LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Network Xml Widgets Gui SerialPort Qml)
qt_standard_project_setup()

add_executable(mconf
	../lib/Configurator.cpp
	../lib/Configurator.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	ApplicationTabPage.cpp
	ApplicationTabPage.h
	DiagTabPage.cpp
	DiagTabPage.h
	ftdi/ftd2xx.h
	main.cpp
	ModuleConfigurator.cpp
	ModuleConfigurator.h
	Settings.cpp
	Settings.h
	SettingsForm.cpp
	SettingsForm.h

	diagtabpage.ui
)

target_precompile_headers(mconf PRIVATE
	<cassert>
	<cstdint>
	<memory>
	<vector>
	<algorithm>
	<functional>

	<QtCore>
	<QtWidgets>
	<QDateEdit>
	<QDebug>
	<QString>
	<QMessageBox>
	<QTabWidget>
	<QFileDialog>
	<QtNetwork/QHostInfo>
	<QSqlDatabase>
	<QSqlError>
	<QSqlDriver>
	<QSqlQuery>
	<QtEndian>
	<QSerialPort>
	<QSerialPortInfo>
)

target_link_libraries(mconf PRIVATE Qt6::Core Qt6::Sql Qt6::Network Qt6::Xml Qt6::Widgets Qt6::Gui Qt6::SerialPort Qt6::Qml)
target_link_libraries(mconf PRIVATE CommonLib Protobuf UtilsLib HardwareLib)

if(MSVC)
	add_library(ftdi STATIC IMPORTED)
	set_target_properties(ftdi PROPERTIES
	  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/mconf/ftdi64/ftd2xx.lib"
	  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/mconf/ftdi"
    )

    target_link_libraries(mconf PRIVATE ftdi)
endif()

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(mconf PRIVATE vld.lib)
endif()

