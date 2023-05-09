cmake_minimum_required(VERSION 3.16)

project(CfgSrv LANGUAGES CXX)

include(../vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Qml Xml Widgets)
qt_standard_project_setup()

add_executable(CfgSrv
	../CommonLib/Types.h
	CfgChecker.cpp
	CfgChecker.h
	CfgControlServer.cpp
	CfgControlServer.h
	CfgServiceMain.cpp
	ConfigurationService.cpp
	ConfigurationService.h
)

target_precompile_headers(CfgSrv PRIVATE
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
	<QtCore>
	<QtGlobal>
	<QtNetwork/QHostAddress>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>
)

target_link_libraries(CfgSrv PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Qml Qt6::Xml Qt6::Widgets)
target_link_libraries(CfgSrv PUBLIC CommonLib Protobuf UtilsLib AppSignalLib HardwareLib ServiceLib OnlineLib ClientLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(CfgSrv PRIVATE vld.lib)
endif()
