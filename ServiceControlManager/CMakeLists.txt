cmake_minimum_required(VERSION 3.16)

project(scm LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Qml Xml Widgets)
qt_standard_project_setup()

set(TARGET scm)

set (SCM_RESOURCES
	../Etc/Logo/RadiyLogo.qrc
)

add_executable(${TARGET} WIN32
	${SCM_RESOURCES}
	AppDataServiceWidget.cpp
	AppDataServiceWidget.h
	AppDataSourceWidget.cpp
	AppDataSourceWidget.h
	AppDataSourcesModel.cpp
	AppDataSourcesModel.h
	BaseServiceWidget.cpp
	BaseServiceWidget.h
	CfgServiceWidget.cpp
	CfgServiceWidget.h
	Columns.cpp
	Columns.h
	MainWindow.cpp
	MainWindow.h
	ScanOptionsWidget.cpp
	ScanOptionsWidget.h
	ScmMain.cpp
	ScmServiceClient.cpp
	ScmServiceClient.h
	ScmTcpAppDataClient.cpp
	ScmTcpAppDataClient.h
	ServiceData.cpp
	ServiceData.h
	ServiceTableModel.cpp
	ServiceTableModel.h
	TcpTuningServiceClient.cpp
	TcpTuningServiceClient.h
	TuningServiceWidget.cpp
	TuningServiceWidget.h
	TuningSourceWidget.cpp
	TuningSourceWidget.h

	ServiceControlManager.qrc
	translations/ServiceControlManager_ru.ts
	translations/ServiceControlManager_uk.ts
)

target_precompile_headers(${TARGET} PRIVATE
	# C includes, must be before c++ includes
	#
	<cassert>
	<cstdint>

	# C++ includes
	#
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

	# Qt Includes
	#
	<QAbstractSocket>
	<QAction>
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
	<QHeaderView>
	<QHostAddress>
	<QHBoxLayout>
	<QMainWindow>
	<QMap>
	<QMessageBox>
	<QMetaClassInfo>
	<QMetaEnum>
	<QMetaObject>
	<QMutex>
	<QMutexLocker>
	<QLabel>
	<QObject>
	<QQueue>
	<QSettings>
	<QSortFilterProxyModel>
	<QSplitter>
	<QStandardItemModel>
	<QStatusBar>
	<QString>
	<QStringList>
	<QTableView>
	<QTextStream>
	<QThread>
	<QTimer>
	<QToolBar>
	<QTranslator>
	<QtGlobal>
	<QtNetwork/QHostAddress>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	<CommonLib/ConstStrings.h>
	<CommonLib/Types.h>
	<CommonLib/Hash.h>

	"../UtilsLib/Ui/WidgetUtils.h"
	
	<Network.pb.h>

	"../AppSignalLib/AppSignal.h"
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Qml Qt6::Xml Qt6::Widgets)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib HardwareLib ServiceLib OnlineLib ClientLib UiLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
