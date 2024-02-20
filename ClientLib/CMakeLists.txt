cmake_minimum_required(VERSION 3.16)

project(ClientLib LANGUAGES CXX)

include(../cmake/translations.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Xml Network Widgets LinguistTools Qml)

set(TARGET ClientLib)

qt_add_library(${TARGET} STATIC
	AdsConnection.cpp
	AdsConnection.h
	AdsSourceStateConnection.cpp
	AdsSourceStateConnection.h
	AppSignalManager.cpp
	AppSignalManager.h
	ClientTranslator.cpp
	ClientTranslator.h
	ConfigController.cpp
	ConfigController.h
	IAppSignalUpdater.h
	IRecentAppSignals.h
	ITestObserver.h
	ITuningLog.h
	RtDataProvider.cpp
	RtDataProvider.h
	RtTrendTcpClient.cpp
	RtTrendTcpClient.h
	ScriptTestObserver.cpp
	ScriptTestObserver.h
	TcpAppSourcesState.cpp
	TcpAppSourcesState.h
	TcpSignalClient.cpp
	TcpSignalClient.h
	TcpSignalRecents.cpp
	TcpSignalRecents.h
	TuningConnection.cpp
	TuningConnection.h
	TuningLog.cpp
	TuningLog.h
	TuningSourceState.cpp
	TuningSourceState.h
	TuningTcpClient.cpp
	TuningTcpClient.h
	TuningUserManager.cpp
	TuningUserManager.h
	TuningWriteCommand.cpp
	TuningWriteCommand.h
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
	<QPushButton>
	<QReadWriteLock>
	<QSharedMemory>
	<QThread>
	<QTimeZone>
	<QTranslator>
	<QVBoxLayout>
	<QXmlStreamReader>
	<QXmlStreamWriter>

	"../CommonLib/Types.h"
	"../CommonLib/Hash.h"
	"../CommonLib/PropertyObject.h"

	"../Proto/ProtoSerialization.h"
	"../Proto/Network.pb.h"	
	#"../Proto/AppSignal.pb.h"

	"../AppSignalLib/AppSignalParam.h"
	"../AppSignalLib/AppSignalState.h"
	"../AppSignalLib/TuningValue.h"

	"../OnlineLib/Tcp.h"
)

#Add translations
#
add_translation(${TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_link_libraries(${TARGET} PUBLIC Qt6::Gui Qt6::Xml Qt6::Network Qt6::Widgets Qt6::Qml)
target_link_libraries(${TARGET} PUBLIC CommonLib Protobuf UtilsLib OnlineLib AppSignalLib TrendView)

if (MSVC)
	target_link_libraries(${TARGET} PUBLIC Advapi32)
endif (MSVC)
if (UNIX)
	target_link_libraries(${TARGET} PUBLIC pam pam_misc)
endif (UNIX)

target_compile_definitions(${TARGET} PRIVATE CLIENT_LIB_DOMAIN)
#target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_code_coverage_options(${TARGET} CODE_COVERAGE)
