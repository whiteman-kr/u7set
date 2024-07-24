cmake_minimum_required(VERSION 3.16)

project(mconf LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Network Xml Widgets Gui SerialPort Qml)
qt_standard_project_setup()

set(TARGET mconf)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

add_executable(${TARGET} WIN32
	../lib/Configurator.cpp
	../lib/Configurator.h
	ApplicationTabPage.cpp
	ApplicationTabPage.h
	DiagTabPage.cpp
	DiagTabPage.h
	ftdi/ftd2xx.h
	Globals.cpp
	Globals.h
	main.cpp
	MconfSettings.cpp
	MconfSettings.h
	ModuleConfigurator.cpp
	ModuleConfigurator.h
	SettingsForm.cpp
	SettingsForm.h

	../Etc/Logo/RadiyLogo.qrc

	diagtabpage.ui
	${WINDOWS_RESOURCES}
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<cassert>
	<cstdint>
	<functional>
	<memory>
	<vector>

	<QApplication>
	<QComboBox>
	<QDateEdit>
	<QDebug>
	<QFileDialog>
	<QGridLayout>
	<QHBoxLayout>
	<QLabel>
	<QLineEdit>
	<QMessageBox>
	<QPushButton>
	<QSerialPort>
	<QSerialPortInfo>
	<QSettings>
	<QSplitter>
	<QSqlDatabase>
	<QSqlDriver>
	<QSqlError>
	<QSqlQuery>
	<QString>
	<QTabWidget>
	<QtEndian>
	<QTextEdit>
	<QThread>
	<QtNetwork/QHostInfo>
	<QTreeWidget>
	<QTreeWidgetItem>
	<QUuid>
	<QVBoxLayout>

	# Project includes
	#
	<CommonLib/Types.h>
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Sql Qt6::Network Qt6::Xml Qt6::Widgets Qt6::Gui Qt6::SerialPort Qt6::Qml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib HardwareLib UiLib)

if(MSVC)
	add_library(ftdi STATIC IMPORTED)
	set_target_properties(ftdi PROPERTIES
	  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/ftdi64/ftd2xx.lib"
	  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/ftdi"
    )

    target_link_libraries(${TARGET} PRIVATE ftdi)
endif()

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

