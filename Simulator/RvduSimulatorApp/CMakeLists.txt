cmake_minimum_required(VERSION 3.16)

project(RvduSimulatorApp LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

set(TARGET RvduSimulatorApp)

set (TC_SOURCES
	main.cpp
    MainWindow.cpp
    MainWindow.h
    RvduSimThread.cpp
    RvduSimThread.h
    RvduWidget.cpp
    RvduWidget.h
	SignalsView.cpp
	SignalsView.h
	SharedMemoryView.cpp
	SharedMemoryView.h
	SimulatorCommThread.cpp
	SimulatorCommThread.h
)

set (TC_RESOURCES
#	Resources.qrc
#	../Etc/Logo/RadiyLogo.qrc
)

set (TC_FORMS
	
)

add_executable(${TARGET} WIN32
	${TC_FORMS}
	${TC_SOURCES}
	${TC_RESOURCES}
	${WINDOWS_RESOURCES}
)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# Qt includes
	#
	<QTableView>
	<QHeaderView>
	<QInputDialog>
	<QMessageBox>
	<QSettings>
	<QTimer>
	<QApplication>
	<QHBoxLayout>
	<QLabel>
	<QMenuBar>
	<QPushButton>
	<QStatusBar>
	<QVBoxLayout>
	<QWidget>
	<QThread>
	<QDate>
	<QUuid>

	
)

target_compile_definitions(${TARGET} PRIVATE RVDU_SIM)


target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Widgets)
target_link_libraries(${TARGET} PRIVATE CommonLib UiLib LicenseLib RvduSimulatorLib)

#add_code_coverage_options(${TARGET} CODE_COVERAGE)
