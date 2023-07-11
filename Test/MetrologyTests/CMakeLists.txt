cmake_minimum_required(VERSION 3.16)

project(MetrologyTests LANGUAGES CXX)

enable_testing()

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
	../../Metrology/UnitsConvertor.cpp
	../../Metrology/UnitsConvertor.h
	../../Metrology/UnitsConvertorTable.cpp
	../../Metrology/UnitsConvertorTable.h

	MetrologyFormulaTests.cpp
	MetrologyFormulaTests.h
	UnitsConvertorTests.cpp
	UnitsConvertorTests.h
)

target_precompile_headers(${TARGET} PRIVATE
	<QVector>
	<QtCore>
	<QHostAddress>
)

target_link_libraries(${TARGET} PRIVATE Qt6::Test Qt6::Core Qt6::Network Qt6::Qml)
target_link_libraries(${TARGET} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
