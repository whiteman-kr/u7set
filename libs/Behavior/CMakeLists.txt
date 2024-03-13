cmake_minimum_required(VERSION 3.16)

project(Behavior LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Gui)

set(TARGET Behavior)

qt_add_library(${TARGET} STATIC
	./include/Behavior/ClientBehavior.h
	./include/Behavior/ClientBehaviorStorage.h
	./include/Behavior/MonitorBehavior.h
	./include/Behavior/ScriptMonitorBehavior.h
	./include/Behavior/TuningClientBehavior.h
	./include/Behavior/BehaviorColorPair.h

	./src/ClientBehavior.cpp
	./src/ClientBehaviorStorage.cpp
	./src/MonitorBehavior.cpp
	./src/ScriptMonitorBehavior.cpp
	./src/TuningClientBehavior.cpp
)

target_precompile_headers(${TARGET} PRIVATE
		<memory>
		<vector>
		
		<QColor>
		<QXmlStreamReader>
		<QXmlStreamWriter>
		
		<QObject>
		<QString>

		<Types.h>
		<Hash.h>
		<PropertyObject.h>
)

target_link_libraries(${TARGET} PUBLIC Qt6::Gui)
target_link_libraries(${TARGET} PUBLIC CommonLib)

target_compile_definitions(${TARGET} PRIVATE BEHAVIOR_LIBRARY)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_code_coverage_options(${TARGET} CODE_COVERAGE)
