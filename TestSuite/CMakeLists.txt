cmake_minimum_required(VERSION 3.16)

project(TestSuite LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Qml Sql Xml Test)
qt_standard_project_setup()

set(TARGET_NAME TestSuite)

add_executable(${TARGET_NAME}
	../lib/CodeEditor.cpp
	../lib/CodeEditor.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	../lib/Ui/DialogAlert.cpp
	../lib/Ui/DialogAlert.h
	../lib/Ui/DialogTcpStatistics.cpp
	../lib/Ui/DialogTcpStatistics.h
	../lib/Ui/TabWidgetEx.cpp
	../lib/Ui/TabWidgetEx.h
	AppConfigSettings.cpp
	AppConfigSettings.h
	AppLogOutputWidget.cpp
	AppLogOutputWidget.h
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
)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_precompile_headers(${TARGET_NAME} PRIVATE
	<cassert>
	<cstdint>
	<memory>
	<vector>
	<algorithm>
	<functional>

	<QtCore>
	<QtGui>
	<QtWidgets>
	<QtNetwork>
	<QtQml>
	<QtSql>
	<QtXml>
	<QtTest>
)

target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Sql Qt6::Xml Qt6::Test)
target_link_libraries(${TARGET_NAME} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib TestSuiteLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET_NAME} PRIVATE vld.lib)
endif()

