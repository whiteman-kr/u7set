cmake_minimum_required(VERSION 3.16)

project(MetrologyTests LANGUAGES CXX)
message("${PROJECT_NAME}, Version: ${CMAKE_PROJECT_VERSION}")

enable_testing()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Test Core Network)

set(TARGET_NAME MetrologyTests)

add_executable(${TARGET_NAME}
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

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_precompile_headers(${TARGET_NAME} PRIVATE
	<QVector>
	<QtCore>
	<QHostAddress>
)

target_link_libraries(${TARGET_NAME} PRIVATE Qt6::Test Qt6::Core Qt6::Network)
target_link_libraries(${TARGET_NAME} PRIVATE CommonLib Protobuf UtilsLib AppSignalLib OnlineLib HardwareLib)

