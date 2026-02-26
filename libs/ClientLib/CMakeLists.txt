cmake_minimum_required(VERSION 3.16)

project(ClientLib LANGUAGES CXX)

include(../../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET ClientLib)

qt_add_library(${TARGET} STATIC
	./include/ClientLib/AdsSourceStateConnection.h
	./include/ClientLib/AppDataSourceState.h
	./include/ClientLib/AppSignalManager.h
	./include/ClientLib/ClientTranslator.h
	./include/ClientLib/ConfigController.h
	./include/ClientLib/ITestObserver.h
	./include/ClientLib/ITuningAuthorization.h
	./include/ClientLib/ITuningConnection.h
	./include/ClientLib/ITuningLog.h
	./include/ClientLib/ITuningSignalUpdater.h
	./include/ClientLib/RtDataProvider.h
	./include/ClientLib/RtTrendConnectionStatistics.h
	./include/ClientLib/ScriptTestObserver.h
	./include/ClientLib/SignalLog.h
	./include/ClientLib/TuningConnection.h
	./include/ClientLib/TuningLog.h
	./include/ClientLib/TuningSignalManager.h
	./include/ClientLib/TuningSourceState.h
	./include/ClientLib/TuningUserManager.h
	./include/ClientLib/TuningWriteCommand.h
	./include/ClientLib/LoggerStdAdapter.h
	./include/ClientLib/ServiceEndpoint.h
	
	./src/AdsSourceStateConnection.cpp
	./src/AdsSourceStateConnectionPrivate.cpp
	./src/AdsSourceStateConnectionPrivate.h
	./src/AdsSourceStateConnectionPrivate2.cpp
	./src/AdsSourceStateConnectionPrivate2.h
	./src/AppDataSourceState.cpp
	./src/AppSignalManager.cpp
	./src/ClientTranslator.cpp
	./src/ConfigController.cpp
	./src/RtDataProvider.cpp
	./src/RtDataProviderPrivate.cpp
	./src/RtDataProviderPrivate.h
	./src/RtTrendTcpClient.cpp
	./src/RtTrendTcpClient.h
	./src/ScriptTestObserver.cpp
	./src/SignalLog.cpp
	./src/TcpAppSourcesState.cpp
	./src/TcpAppSourcesState.h
	./src/TcpSignalClient.cpp
	./src/TcpSignalClient.h
	./src/TcpSignalRecents.cpp
	./src/TcpSignalRecents.h
	./src/TuningConnection.cpp
	./src/TuningConnectionPrivate.cpp
	./src/TuningConnectionPrivate.h
	./src/TuningLog.cpp
	./src/TuningSignalManager.cpp
	./src/TuningSourceState.cpp
	./src/TuningTcpClient.cpp
	./src/TuningTcpClient.h
	./src/TuningUserManager.cpp
	./src/TuningWriteCommand.cpp
)

target_precompile_headers(${TARGET} PRIVATE
	<CommonLib/PchGuard.h>

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

	<QDateTime>
	<QThread>
	<QReadWriteLock>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>

	<ProtoSerialization.h>
	<Network.pb.h>
	<GrpcAppDataSrv.grpc.pb.h>

	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
	"../../AppSignalLib/TuningValue.h"

#	"../../OnlineLib/Tcp.h"
)

target_compile_definitions(${TARGET} PRIVATE CLIENT_LIB_DOMAIN)
target_compile_definitions(${TARGET} PRIVATE USE_GRPC_ADS_CONNECTION)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib UtilsLib OnlineLib AppSignalLib TrendView ProtobufEx AdsConnectionLib)

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
