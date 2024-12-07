cmake_minimum_required(VERSION 3.16)

project(SchemaClientLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml PrintSupport)

set(TARGET SchemaClientLib)

qt_add_library(${TARGET} STATIC
	./include/SchemaClientLib/AppDataSourcesWidget.h
	./include/SchemaClientLib/ClientSchemaManager.h
	./include/SchemaClientLib/DevToolsWindow.h
	./include/SchemaClientLib/DialogSignalInfo.h
	./include/SchemaClientLib/DialogSignalSearch.h
	./include/SchemaClientLib/DialogSignalSnapshot.h
	./include/SchemaClientLib/DialogTcpStatistics.h
	./include/SchemaClientLib/DialogWriteTuningValues.h
	./include/SchemaClientLib/DragDropHelper.h
	./include/SchemaClientLib/IDevTools.h
	./include/SchemaClientLib/SchemaClientConfigController.h
	./include/SchemaClientLib/SchemaDrawStatistics.h
	./include/SchemaClientLib/SchemaListWidget.h
	./include/SchemaClientLib/SchemaTabWidget.h
	./include/SchemaClientLib/TuningSourcesWidget.h
	AppDataSourcesWidget.cpp
	ClientSchemaManager.cpp
	DevToolsAppSettings.cpp
	DevToolsAppSettings.h
	DevToolsConnections.cpp
	DevToolsConnections.h
	DevToolsGlobalScript.cpp
	DevToolsGlobalScript.h
	DevToolsSchemaStats.cpp
	DevToolsSchemaStats.h
	DevToolsScriptVariables.cpp
	DevToolsScriptVariables.h
	DevToolsViewVariables.cpp
	DevToolsViewVariables.h
	DevToolsWidget.cpp
	DevToolsWidget.h
	DevToolsWindow.cpp
	DialogSignalInfo.cpp
	DialogSignalInfo.ui
	DialogSignalSearch.cpp
	DialogSignalSnapshot.cpp
	DialogSourceInfo.cpp
	DialogSourceInfo.h
	DialogTcpStatistics.cpp
	DialogWriteTuningValues.cpp
	DragDropHelper.cpp
	SchemaClientConfigController.cpp
	SchemaDrawStatistics.cpp
	SchemaListWidget.cpp
	SchemaListWidgetPrivate.cpp
	SchemaListWidgetPrivate.h
	SchemaTabWidget.cpp
	SignalSnapshotModel.cpp
	SignalSnapshotModel.h
	SignalSnapshotWidget.cpp
	SignalSnapshotWidget.h
	TcpStatisticsWidget.cpp
	TcpStatisticsWidget.h
	TuningSourcesWidget.cpp
)

# These precompiled headers are reusend in client projects e.g. Monitor, Diagonstics, TuningClient, TestSuite etc.
target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
	<algorithm>
	<array>
	<atomic>
	<cassert>
	<chrono>
	<concepts>
	<cstdint>
	<deque>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<mutex>
	<queue>
	<ranges>
	<set>
	<stdexcept>
	<type_traits>
	<unordered_map>
	<unordered_set>
	<vector>

	<QAbstractItemModel>
	<QAbstractSocket>
	<QAbstractTableModel>
	<QAction>
	<QActionGroup>
	<QApplication>
	<QButtonGroup>
	<QCheckBox>
	<QClipboard>
	<QCloseEvent>
	<QColorDialog>
	<QComboBox>
	<QCommandLineParser>
	<QCompleter>
	<QCoreApplication>
	<QCryptographicHash>
	<QDateTime>
	<QDebug>
	<QDialog>
	<QDialogButtonBox>
	<QDir>
	<QDockWidget>
	<QDomDocument>
	<QDrag>
	<QElapsedTimer>
	<QErrorMessage>
	<QEventLoop>
	<QFile>
	<QFileDialog>
	<QFileInfo>
	<QGroupBox>
	<QHash>
	<QHBoxLayout>
	<QHeaderView>
	<QHostAddress>
	<QInputDialog>
	<QJSEngine>
	<QJsonArray>
	<QJsonDocument>
	<QJsonObject>
	<QKeyEvent>
	<QLabel>
	<QLineEdit>
	<QList>
	<QListView>
	<QListWidget>
	<QLocale>
	<QMainWindow>
	<QMap>
	<QMenu>
	<QMenuBar>
	<QMessageBox>
	<QMetaClassInfo>
	<QMetaEnum>
	<QMetaObject>
	<QMimeData>
	<QMouseEvent>
	<QMutex>
	<QMutexLocker>
	<QObject>
	<QPainter>
	<QProgressBar>
	<QProgressDialog>
	<QPushButton>
	<QQueue>
	<QReadWriteLock>
	<QRegularExpression>
	<QRegularExpressionValidator>
	<QScreen>
	<QScrollArea>
	<QScrollBar>
	<QSettings>
	<QSharedMemory>
	<QSortFilterProxyModel>
	<QSplitter>
	<QStandardPaths>
	<QStatusBar>
	<QString>
	<QStringList>
	<QStringListModel>
	<QTableView>
	<QTableWidget>
	<QTabWidget>
	<QtConcurrent>
	<QTextBrowser>
	<QTextEdit>
	<QTextStream>
	<QThread>
	<QTimer>
	<QTimer>
	<QTimeZone>
	<QtNetwork/QHostAddress>
	<QToolBar>
	<QToolButton>
	<QToolTip>
	<QtPrintSupport>
	<QTranslator>
	<QTreeView>
	<QTreeWidget>
	<QTreeWidgetItem>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVBoxLayout>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	# Project headers
	#
	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	<ProtoSerialization.h>
	<AppSignal.pb.h>
	<Network.pb.h>

	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml Qt6::PrintSupport)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib ProtobufEx OnlineLib AppSignalLib ClientLib VFrame30 UiLib AppSignalLists ReportLib)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
