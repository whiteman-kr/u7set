cmake_minimum_required(VERSION 3.16)

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
	../lib/CodeEditor.cpp
	../lib/CodeEditor.h
	../lib/Ui/AppDataSourcesWidget.cpp
	../lib/Ui/AppDataSourcesWidget.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	../lib/Ui/DialogAlert.cpp
	../lib/Ui/DialogAlert.h
	../lib/Ui/DialogSourceInfo.cpp
	../lib/Ui/DialogSourceInfo.h
	../lib/Ui/DialogTcpStatistics.cpp
	../lib/Ui/DialogTcpStatistics.h
	../lib/Ui/TabWidgetEx.cpp
	../lib/Ui/TabWidgetEx.h
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
	../lib/Ui/TuningSourcesWidget.cpp
	../lib/Ui/TuningSourcesWidget.h
	
	Resources.qrc
	../Etc/Logo/RadiyLogo.qrc

	${WINDOWS_RESOURCES}
	${TC_FORMS}
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<cassert>
	<cstdint>
	<functional>
	<memory>
	<vector>

	<QAction>
	<QComboBox>
	<QCompleter>
	<QDockWidget>
	<QList>
	<QMenu>
	<QMenuBar>
	<QPushButton>
	<QRegularExpression>
	<QStatusBar>
	<QTextBrowser>
	<QTextEdit>
	<QToolBar>
	<QTreeWidget>
	<QTreeWidgetItem>

)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Qml Qt6::Sql Qt6::Xml Qt6::Test)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib ClientLib TestSuiteLib ReportLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()

add_code_coverage_options(${TARGET} CODE_COVERAGE)
