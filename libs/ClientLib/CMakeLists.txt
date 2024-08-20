cmake_minimum_required(VERSION 3.16)

project(ClientLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET ClientLib)

qt_add_library(${TARGET} STATIC
	./include/ClientLib/AdsConnection.h
	./include/ClientLib/AdsSourceStateConnection.h
	./include/ClientLib/AppDataSourceState.h
	./include/ClientLib/AppSignalManager.h
	./include/ClientLib/ClientTranslator.h
	./include/ClientLib/ConfigController.h
	./include/ClientLib/IAppSignalUpdater.h
	./include/ClientLib/IRecentAppSignals.h
	./include/ClientLib/ISignalDataServer.h
	./include/ClientLib/ITestObserver.h
	./include/ClientLib/ITuningAuthorization.h
	./include/ClientLib/ITuningConnection.h
	./include/ClientLib/ITuningLog.h
	./include/ClientLib/ITuningSignalUpdater.h
	./include/ClientLib/RtDataProvider.h
	./include/ClientLib/RtTrendConnectionStatistics.h
	./include/ClientLib/ScriptTestObserver.h
	./include/ClientLib/TuningConnection.h
	./include/ClientLib/TuningLog.h
	./include/ClientLib/TuningSignalManager.h
	./include/ClientLib/TuningSourceState.h
	./include/ClientLib/TuningUserManager.h
	./include/ClientLib/TuningWriteCommand.h
	AdsConnection.cpp
	AdsConnectionPrivate.cpp
	AdsConnectionPrivate.h
	AdsSourceStateConnection.cpp
	AdsSourceStateConnectionPrivate.cpp
	AdsSourceStateConnectionPrivate.h
	AppDataSourceState.cpp
	AppSignalManager.cpp
	ClientTranslator.cpp
	ConfigController.cpp
	RtDataProvider.cpp
	RtDataProviderPrivate.cpp
	RtDataProviderPrivate.h
	RtTrendTcpClient.h
	RtTrendTcpClient.cpp
	ScriptTestObserver.cpp
	TcpAppSourcesState.h
	TcpAppSourcesState.cpp
	TcpSignalClient.cpp
	TcpSignalClient.h
	TcpSignalRecents.h
	TcpSignalRecents.cpp
	TuningConnection.cpp
	TuningConnectionPrivate.cpp
	TuningConnectionPrivate.h
	TuningLog.cpp
	TuningSignalManager.cpp
	TuningSourceState.cpp
	TuningTcpClient.cpp
	TuningTcpClient.h
	TuningUserManager.cpp
	TuningWriteCommand.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<cassert>
	<chrono>
	<cstdint>
	<functional>
	<list>
	<map>
	<memory>
	<mutex>
	<queue>
	<ranges>
	<set>
	<unordered_map>
	<unordered_set>
	<vector>

	<QApplication>
	<QComboBox>
	<QCoreApplication>
	<QDateTime>
	<QDialog>
	<QDomDocument>
	<QHBoxLayout>
	<QJSEngine>
	<QLabel>
	<QLineEdit>
	<QLocale>
	<QMessageBox>
	<QThread>
	<QPushButton>
	<QReadWriteLock>
	<QSharedMemory>
	<QThread>
	<QTimeZone>
	<QTranslator>
	<QVBoxLayout>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	<ProtoSerialization.h>
	<Network.pb.h>

	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"

	"../../OnlineLib/Tcp.h"
)

target_compile_definitions(${TARGET} PRIVATE CLIENT_LIB_DOMAIN)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib ProtobufEx UtilsLib OnlineLib AppSignalLib TrendView)

if (MSVC)
	target_link_libraries(${TARGET} PUBLIC Advapi32)
endif (MSVC)

if (UNIX)
	target_link_libraries(${TARGET} PUBLIC pam pam_misc)
endif (UNIX)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
