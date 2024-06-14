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
	../lib/ConstStrings.h
	DialogChooseFilter.cpp
	DialogChooseFilter.h
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
	TuningClientUiStorage.cpp
	TuningClientUiStorage.h
	TuningConfigController.cpp
	TuningConfigController.h
	TuningSignalListSet.cpp 
	TuningSignalListSet.h
	TreeFilterWidget.cpp
	TreeFilterWidget.h
	TuningPage.cpp
	TuningPage.h
	TuningModel.cpp
	TuningModel.h
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

	<QAbstractItemModel>
	<QAbstractTableModel>
	<QAction>
	<QActionGroup>
	<QApplication>
	<QButtonGroup>
	<QCheckBox>
	<QClipboard>
	<QComboBox>
	<QCommandLineParser>
	<QCryptographicHash>
	<QDebug>
	<QDialog>
	<QDialogButtonBox>
	<QDir>
	<QDomDocument>
	<QFile>
	<QFileDialog>
	<QGridLayout>
	<QHBoxLayout>
	<QHeaderView>
	<QInputDialog>
	<QJsonArray>
	<QJsonDocument>
	<QJsonObject>
	<QKeyEvent>
	<QLabel>
	<QLineEdit>
	<QListView>
	<QListWidget>
	<QMainWindow>
	<QMenu>
	<QMenuBar>
	<QMessageBox>
	<QMouseEvent>
	<QMutex>
	<QMutexLocker>
	<QObject>
	<QPainter>
	<QProcess>
	<QPushButton>
	<QScrollArea>
	<QSettings>
	<QSharedMemory>
	<QSortFilterProxyModel>
	<QSplitter>
	<QStandardPaths>
	<QStatusBar>
	<QTableView>
	<QTableWidget>
	<QTableWidgetItem>
	<QTabWidget>
	<QTextEdit>
	<QTranslator>
	<QTreeWidget>
	<QTreeWidgetItem>
	<QUuid>
	<QVariant>
	<QVBoxLayout>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"
	#"../CommonLib/AfbParamValue.h"
	
	<ProtoSerialization.h>
	<Network.pb.h>
	#<AppSignal.pb.h>
	
	#"../AppSignalLib/AppSignal.h"
	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
	"../AppSignalLib/TuningValue.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

# Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Qml Qt6::Xml Qt6::Svg Qt6::PrintSupport)
target_link_libraries(${TARGET} PRIVATE CommonLib UtilsLib AppSignalLib OnlineLib VFrame30 ClientLib SchemaClientLib TuningLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
