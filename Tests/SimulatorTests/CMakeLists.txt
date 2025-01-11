cmake_minimum_required(VERSION 3.25)

project(SimulatorTests LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml Xml Concurrent)
qt_standard_project_setup()

set(TARGET SimulatorTests)

add_executable(${TARGET}
	main.cpp
	SimAfbParamTests.cpp
	SimAfbParamTests.h
	SimCommandTest_LM5_LM6.cpp
	SimCommandTest_LM5_LM6.h
	SimProfilesTest.cpp
	SimProfilesTest.h
	SimRamTests.cpp
	SimRamTests.h

	Resources.qrc
)


target_precompile_headers(${TARGET} PRIVATE
	<algorithm>
	<array>
	<bit>
	<cassert>
	<cfenv>
	<chrono>
	<cmath>
	<concepts>
	<cstddef>
	<cstring>
	<deque>
	<filesystem>
	<functional>
	<limits>
	<map>
	<memory>
	<optional>
	<queue>
	<ranges>
	<type_traits>
	<unordered_map>
	<variant>
	<vector>

	<QFuture>
	<QJSEngine>
	<QJSValue>
	<QMutex>
	<QObject>
	<QReadWriteLock>
	<QUdpSocket>

	<CommonLib/Types.h>
	<CommonLib/Hash.h>
	<CommonLib/PropertyObject.h>
	<CommonLib/AfbParamValue.h>
	
	<ProtoSerialization.h>
	
	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalParam.h"
	"../../AppSignalLib/AppSignalState.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Qt6::Core Qt6::Network Qt6::Qml Qt6::Xml Qt6::Concurrent)
target_link_libraries(${TARGET} PRIVATE ProtobufEx HardwareLib SimulatorLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
