cmake_minimum_required(VERSION 3.16)

project(TuningClient LANGUAGES CXX)

include(../cmake/vld.cmake)
include(../cmake/translations.cmake)

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

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Network Qml Xml Svg PrintSupport LinguistTools)
qt_standard_project_setup()

set (TC_SOURCES
	../lib/ClientBehavior.cpp
	../lib/ClientBehavior.h
	../lib/ConstStrings.h
	../lib/PropertyEditor.cpp
	../lib/PropertyEditor.h
	../lib/PropertyEditorDialog.cpp
	../lib/PropertyEditorDialog.h
	../lib/Tuning/TuningFilter.cpp
	../lib/Tuning/TuningFilter.h
	../lib/Tuning/TuningFilterEditor.cpp
	../lib/Tuning/TuningFilterEditor.h
	../lib/Tuning/TuningModel.cpp
	../lib/Tuning/TuningModel.h
	../lib/Ui/DialogAbout.cpp
	../lib/Ui/DialogAbout.h
	../lib/Ui/DialogAlert.cpp
	../lib/Ui/DialogAlert.h
	../lib/Ui/DialogSourceInfo.cpp
	../lib/Ui/DialogSourceInfo.h
	../lib/Ui/DialogTcpStatistics.cpp
	../lib/Ui/DialogTcpStatistics.h
	../lib/Ui/TuningSourcesWidget.cpp
	../lib/Ui/TuningSourcesWidget.h
	../lib/Ui/DialogWriteValues.cpp
	../lib/Ui/DialogWriteValues.h
	DialogChooseFilter.cpp
	DialogChooseFilter.h
	DialogFilterEditor.cpp
	DialogFilterEditor.h
	DialogProperties.cpp
	DialogProperties.h
	DialogSettings.cpp
	DialogSettings.h
	DialogTuningSources.cpp
	DialogTuningSources.h
	LogonWorkspace.cpp
	LogonWorkspace.h
	Main.cpp
	Main.h
	MainWindow.cpp
	MainWindow.h
	SchemasWorkspace.cpp
	SchemasWorkspace.h
	ScriptTuningClientApplication.cpp
	ScriptTuningClientApplication.h
	Settings.cpp
	Settings.h
	SwitchFiltersPage.cpp
	SwitchFiltersPage.h
	SwitchFiltersPageOptions.cpp
	SwitchFiltersPageOptions.h
	TuningClientFilterStorage.cpp
	TuningClientFilterStorage.h
	TuningConfigController.cpp
	TuningConfigController.h
	TreeFilterWidget.cpp
	TreeFilterWidget.h
	TuningPage.cpp
	TuningPage.h
	TuningSchemaManager.cpp
	TuningSchemaManager.h
	TuningSchemaView.cpp
	TuningSchemaView.h
	TuningSchemaWidget.cpp
	TuningSchemaWidget.h
	TuningSignalInfo.cpp
	TuningSignalInfo.h
	TuningSourcesHelper.cpp
	TuningSourcesHelper.h
	TuningWorkspace.cpp
	TuningWorkspace.h
)

set (TC_RESOURCES
	Resources.qrc
	../Etc/Logo/RadiyLogo.qrc
)

set (TC_FORMS
	DialogSettings.ui
	SwitchFiltersPageOptions.ui
	TuningSignalInfo.ui
)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

set(TARGET TuningClient)

add_executable(${TARGET} WIN32
	${TC_FORMS}
	${TC_SOURCES}
	${TC_RESOURCES}
	${WINDOWS_RESOURCES}
)

target_precompile_headers(${TARGET} PRIVATE
	<memory>
	<algorithm>
	<set>
	<cassert>

	<QObject>
	<QVector>
	<QVariant>
	<QSplitter>
	<QTreeWidget>
	<QTabWidget>
	<QHBoxLayout>
	<QVBoxLayout>
	<QMessageBox>
	<QListView>
	<QPushButton>
	<QLineEdit>
	<QCheckBox>
	<QComboBox>
	<QTableView>
	<QAbstractItemModel>
	<QAbstractTableModel>
	<QSortFilterProxyModel>
	<QHeaderView>
	<QSettings>
	<QXmlStreamReader>
	<QXmlStreamWriter>
	<QFile>
	<QDebug>
	<QLabel>
	<QStatusBar>
	<QApplication>
	<QMutex>
	<QMutexLocker>
	<QDomDocument>
	<QJsonObject>
	<QJsonDocument>
	<QJsonArray>
	<QAction>
	<QMenu>
	<QMenuBar>
	<QButtonGroup>
	<QUuid>
	<QStandardPaths>
	<QDir>
	<QDialogButtonBox>
	<QTextEdit>
	<QClipboard>
	<QSharedMemory>
	<QCryptographicHash>
	<QPainter>
	<QScrollArea>
	<QMouseEvent>
	<QtQml>
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

# Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Qml Qt6::Xml Qt6::Svg Qt6::PrintSupport)
target_link_libraries(${TARGET} PRIVATE CommonLib UtilsLib AppSignalLib OnlineLib VFrame30 ClientLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
