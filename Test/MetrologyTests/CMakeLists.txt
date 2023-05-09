cmake_minimum_required(VERSION 3.16)

project(MetrologyTests LANGUAGES CXX)

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Network)

add_executable(MetrologyTests
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

target_precompile_headers(Builder PRIVATE
	<QVector>
	<QtCore>
	<QtSql>
	<QHostAddress>
)

target_link_libraries(MetrologyTests PRIVATE Qt6::Test Qt6::Core Qt6::Network)
target_link_libraries(MetrologyTests PUBLIC CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib)

