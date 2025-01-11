cmake_minimum_required(VERSION 3.28)

project(TestSuite LANGUAGES CXX)

include(../cmake/vld.cmake)
include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Qml Sql Xml Test LinguistTools)
qt_standard_project_setup()

set(TARGET TestSuite)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set (TC_FORMS
	DialogReport.ui
	TestSuiteDialogSettings.ui 
)

add_executable(${TARGET} WIN32
	AppConfigSettings.cpp
	AppConfigSettings.h
	AppLogOutputWidget.cpp
	AppLogOutputWidget.h
	DialogDataSources.cpp
	DialogDataSources.h
	DialogReport.cpp
	DialogReport.h
	main.cpp
	TestListWidget.cpp
	TestListWidget.h
	TestLogTabPage.cpp
	TestLogTabPage.h
	TestSuiteDialogSettings.cpp
	TestSuiteDialogSettings.h
	TestSuiteLog.cpp
	TestSuiteMainWindow.cpp
	TestViewTabPage.cpp
	
	Resources.qrc
	../Etc/Logo/RadiyLogo.qrc

	${WINDOWS_RESOURCES}
	${TC_FORMS}
)

# Reuse precompiled headers from SchemaClientLib
#
target_precompile_headers(${TARGET} REUSE_FROM SchemaClientLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Sql Qt6::Xml Qt6::Test)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib OnlineLib ClientLib SchemaClientLib TestSuiteLib ReportLib UiLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
