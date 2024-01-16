cmake_minimum_required(VERSION 3.16)

project(DiagDataSrv LANGUAGES CXX)

include(../cmake/vld.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets Qml Xml)
qt_standard_project_setup()


set(TARGET DiagDataSrv)

add_executable(${TARGET}
	../lib/DataSource.cpp
	../lib/DataSource.h
	DiagDataReceiver.cpp
	DiagDataReceiver.h
	DiagDataService.cpp
	DiagDataService.h
	DiagDataServiceMain.cpp
	DiagDataSource.cpp
	DiagDataSource.h
	DynamicDiagSignalState.cpp
	DynamicDiagSignalState.h
	SignalStatesProcessingThread.cpp
	SignalStatesProcessingThread.h
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
	<QtGlobal>
	<QtNetwork/QHostAddress>
	<QUdpSocket>
	<QUuid>
	<QVariant>
	<QVector>
	<QXmlStreamReader>
	<QXmlStreamWriter>
)

target_include_directories(${TARGET} PRIVATE ${CMAKE_BINARY_DIR}) # for generated files, like version.h

target_include_directories(${TARGET} PRIVATE ./../asio/include)
target_compile_definitions(${TARGET} PRIVATE ASIO_STANDALONE=1)

target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Qml Qt6::Xml)
target_link_libraries(${TARGET} PRIVATE HardwareLib AppSignalLib OnlineLib ServiceLib)

# Link Visaul Leak Detector
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND MSVC)
	target_link_libraries(${TARGET} PRIVATE vld.lib)
endif()
