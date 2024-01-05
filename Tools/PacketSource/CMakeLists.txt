cmake_minimum_required(VERSION 3.16)

project(PacketSource LANGUAGES CXX)

include(../../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Sql Qml Xml Concurrent)
qt_standard_project_setup()

set(TARGET PacketSource)

# Add icon
#
if (MSVC)
	set (WINDOWS_RESOURCES ./WinResources/WinResources.rc)
endif (MSVC)

add_executable(${TARGET} WIN32
	../../lib/DataSource.cpp
	../../lib/DataSource.h
	../../lib/Ui/DialogAbout.cpp
	../../lib/Ui/DialogAbout.h
	BuildOption.cpp
	BuildOption.h
	CmdLineParam.h
	ConfigSocket.cpp
	ConfigSocket.h
	FindSignalPanel.cpp
	FindSignalPanel.h
	FrameBase.cpp
	FrameBase.h
	FrameDataPanel.cpp
	FrameDataPanel.h
	History.cpp
	History.h
	HistoryList.cpp
	HistoryList.h
	main.cpp
	MainWindow.cpp
	MainWindow.h
	Options.cpp
	Options.h
	OptionsDialog.cpp
	OptionsDialog.h
	PacketSourceCore.cpp
	PacketSourceCore.h
	SignalBase.cpp
	SignalBase.h
	SignalList.cpp
	SignalList.h
	SourceBase.cpp
	SourceBase.h
	SourceList.cpp
	SourceList.h
	SourceWorker.cpp
	SourceWorker.h
	UalTesterServer.cpp
	UalTesterServer.h

	resources.qrc
	../../Etc/Logo/RadiyLogo.qrc
	${WINDOWS_RESOURCES}
)

target_precompile_headers(${TARGET} PRIVATE
	# C/C++ includes
	#
	<cassert>
	<cstdint>
	<algorithm>
	<atomic>
	<functional>
	<iostream>
	<limits>
	<list>
	<map>
	<memory>
	<vector>
	<set>
	<type_traits>

	# Qt includes
	#
	<QAbstractSocket>
	<QApplication>
	<QCommandLineParser>
	<QCoreApplication>
	<QDebug>
	<QDateTime>
	<QElapsedTimer>
	<QEventLoop>
	<QFile>
	<QFileInfo>
	<QHash>
	<QHostAddress>
	<QMap>
	<QMetaClassInfo>
	<QMetaEnum>
	<QMetaObject>
	<QMutex>
	<QMutexLocker>
	<QObject>
	<QQueue>
	<QSettings>
	<QString>
	<QStringList>
	<QTextStream>
	<QThread>
	<QTimer>
	<QTranslator>
	<QtConcurrent/QtConcurrent>
	<QtGlobal>
	<QtNetwork/QHostAddress>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	# Qt widgets
	#
	<QAbstractItemModel>
	<QAbstractTableModel>
	<QAction>
	<QCheckBox>
	<QClipboard>
	<QCloseEvent>
	<QErrorMessage>
	<QFileDialog>
	<QHBoxLayout>
	<QHeaderView>
	<QInputDialog>
	<QLabel>
	<QListView>
	<QListWidget>
	<QMainWindow>
	<QMenu>
	<QMenuBar>
	<QMessageBox>
	<QVBoxLayout>
	<QPainter>
	<QProgressDialog>
	<QPushButton>
	<QScrollArea>
	<QScrollBar>
	<QSortFilterProxyModel>
	<QSplitter>
	<QStatusBar>
	<QTableView>
	<QTableWidget>
	<QTabWidget>
	<QTextEdit>
	<QToolBar>
	<QTreeWidget>
	<QVBoxLayout>

	# Project includes
	#
	"../../Proto/ProtoSerialization.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Sql Qt6::Qml Qt6::Xml Qt6::Concurrent)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib )

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	 target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
