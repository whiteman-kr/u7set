cmake_minimum_required(VERSION 3.28)

project(MetrologyTests LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Network Qml)
qt_standard_project_setup()

set(TARGET MetrologyTests)

add_executable(${TARGET}
	main.cpp

	../../Metrology/MetrologyFormula.cpp
	../../Metrology/MetrologyFormula.h
	../../Metrology/UnitsConverter.cpp
	../../Metrology/UnitsConverter.h
	../../Metrology/UnitsConverterTable.cpp
	../../Metrology/UnitsConverterTable.h

	MetrologyFormulaTests.cpp
	MetrologyFormulaTests.h
	UnitsConverterTests.cpp
	UnitsConverterTests.h
)

target_precompile_headers(${TARGET} PRIVATE
	<map>
	<set>

	<QVector>
	<QHostAddress>

	# Project includes
	#
	<CommonLib/Types.h>
	<CommonLib/Hash.h>

	<ProtoSerialization.h>

	"../../AppSignalLib/AppSignal.h"
	"../../AppSignalLib/AppSignalState.h"
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Qt6::Core Qt6::Network Qt6::Qml)
target_link_libraries(${TARGET} PRIVATE CommonLib ProtobufEx UtilsLib AppSignalLib OnlineLib HardwareLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
